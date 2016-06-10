#include "metadataencoder.hpp"


void
MetaDataEncoder::metadata_separators(std::string metadata)
{
    sep.clear();
    for (std::string::iterator it = metadata.begin(); it != metadata.end(); ++it)
    {
        if (*it == ' ' || *it == '.' || *it == '-' || *it == ':' || *it == '#')
            sep.push_back(*it);
    }
}


void
MetaDataEncoder::metadata_analyze(std::vector<read_t>& reads, uint32_t entries)
{   
    std::string metadata(reads[0].meta_data);
    metadata.erase(0,1);
    metadata_separators(metadata);    // List of separators in order  
    std::vector<std::string> parts;
    parts.clear();
    EncodeUtil::split(metadata, parts);
    num_fields = parts.size();
    num_sep = sep.size();
    std::cout << "  - Analyzing Fields: [" << unsigned(num_fields) << "]\n";
    
    // Initialize set of values for each field 
    // to count the number of total values
    std::vector<std::set<std::string> > values;
    for (int i = 0; i < num_fields; i++)
        values.push_back(std::set<std::string>());
    {
        std::vector<std::string> v;
        EncodeUtil::split(metadata, v);
        for (int i = 0; i < num_fields; i++)
            values[i].insert(v[i]);
    }

    for (auto it = ++reads.begin(); it != reads.end(); it++)
    {
        std::vector<std::string> v;
        metadata = std::string(it->meta_data);
        metadata.erase(0,1);
        EncodeUtil::split(metadata, v);
        for (int i = 0; i < num_fields; i++) {
            values[i].insert(v[i]);      
        }
    }

    for (auto it = values.begin(); it != values.end(); it++)
    {

        if (it->size() == 1)
        {
            std::cout << "    - Constant Alphanumeric: " << *it->begin() << std::endl;
            fields.push_back(new ConstantAlphanumericFieldEncoder(b, *it->begin()));
        }
        else if (it->size() > 0)
        {
            uint32_t max = 0;
            uint32_t min = 1000000000;
            bool numeric = true;
            for (auto sit = it->begin(); sit != it->end(); sit++)
            {
                
                if (!numeric || !EncodeUtil::is_numeric(*sit))
                {
                    if (numeric && !EncodeUtil::is_numeric(*sit))
                    {
                        max = 0;
                    }
                    numeric = false;
                    //max = EncodeUtil::ceil_log(max, 10);
                    max = max > sit->length() ? max : sit->length();
                }
                else
                { 
                    uint32_t val = atoi(sit->c_str());
                    max = max > val ? max : val;                     
                    min = min < val ? min : val;
                }
            }
            if (numeric)
            {
                //std::cout << "Max: " << max << "  Min: " << min << "  Entries: " << entries << std::endl;
                if (max - min + 1 == entries)
                {
                    std::cout << "    - Auto Incrementing" << std::endl;    
                    fields.push_back(new AutoIncrementingFieldEncoder(b, 1));
                }
                else 
                {
                    std::cout << "    - Numeric  (Values: " << it->size() << " |  Max: " << max << ")\n";
                    // Non-incremental
                    uint32_t wd = EncodeUtil::ceil_log(max + 1, 2);
                    fields.push_back(new NumericFieldEncoder(b, wd, false));
                }
            }        
            else 
            {
                std::cout << "    - Alphanumeric  (Values: " << it->size() << ")\n";
                int bits_per_char = 8;
                if (it->size() * 100 < entries) // Enable mapping
                {
                    fields.push_back(new AlphanumericFieldEncoder(b, it->size(), true, *it));
                }
                else 
                {
                    printf("Max: %u\n", max);
                    fields.push_back(new AlphanumericFieldEncoder(b, max * bits_per_char, false, *it));
                }
            }
        }
    }
}



void
MetaDataEncoder::encode_separators(void)
{
    static const int bits = 3;
    for (auto it = sep.begin(); it != sep.end(); it++)
    {
        switch (*it)
        {
            case ' ':
                b->write(0, bits);
                break;
            case '.':
                b->write(1, bits);
                break;
            case ':':
                b->write(2, bits);
                break;
            case '-':
                b->write(3, bits);
                break;
            case '#':
                b->write(4, bits);
                break;   
        }
    }
}


void 
MetaDataEncoder::decode_separators(void)
{
    const char* smap = " .:-#";
    // Number of separators = num_sep
    sep.clear();
    sep.push_back('@');
    for (int i = 0; i < num_sep; i++) 
        sep.push_back(smap[b->read(3)]);
}


bool
compress_parallel(std::vector<read_t>::iterator begin, std::vector<read_t>::iterator end, std::vector<MetadataFieldEncoder*> fencs, std::shared_ptr<std::vector<bb_entry_t> > comp_entries)
{
    std::vector<MetadataFieldEncoder*> fencoders;
    
    // Clone all field encoders
    for (MetadataFieldEncoder* fenc : fencs)
        fencoders.push_back(fenc->clone());

    // Set field encoders to return tuples of compressed values to bb_entry    
    for (MetadataFieldEncoder* mfep : fencoders) {
        if (mfep)
            mfep->set_encoded(comp_entries);
    }
    
    // Compress values
    bool fail = false;
    int num_fields = fencoders.size();
    std::string metadata;
    for (auto it = begin; it != end; it++)
    {
        metadata = std::string(it->meta_data);
        metadata.erase(0,1);
        std::vector<std::string> v;
        EncodeUtil::split(metadata, v);       
        for (int i = 0; i < num_fields; i++)
            if (!fencoders[i]->encode(v[i])) {
                fail = true;
                //printf("Error at %dth encoder\n - Metadata:%s\n", i, metadata.c_str());
            }
        if (fail) {
            break;
        }
    }
    
    for (MetadataFieldEncoder* fenc : fencoders)
        delete fenc;
    return !fail;
}


void
MetaDataEncoder::decode_fields(void)
{
    fields.clear();
    for (uint8_t i = 0; i < num_fields; i++)
    {
        MetadataFieldEncoder *enc;
        int fieldtype = b->read(2);
        switch (fieldtype)
        {
            case 0: 
            {
                enc = new ConstantAlphanumericFieldEncoder(b);
                enc->decode_metadata();
                fields.push_back(enc);
                break; 
            }
            case 1:
            {
                enc = new AlphanumericFieldEncoder(b);
                enc->decode_metadata();
                fields.push_back(enc);
                break;
            }
            case 2:
            {
                enc = new NumericFieldEncoder(b);
                enc->decode_metadata();
                fields.push_back(enc);
                break;
            }
            case 3:
            {
                enc = new AutoIncrementingFieldEncoder(b, total_entries);
                enc->decode_metadata();
                fields.push_back(enc);
                break;
            }
        }
    }    
}


void
MetaDataEncoder::decode_entry(read_t& read)
{
    char *md = read.meta_data;
    for (uint8_t i = 0; i < num_fields; i++)
    { 
        *(md++) = sep[i];
        md = fields[i]->decode(md);   
    }
    if (num_sep == num_fields)
        *(md++) = sep[num_sep - 1];
    *md = '\0';
}


MetaDataEncoder::MetaDataEncoder(char *filename) : b(std::shared_ptr<BitBuffer>(new BitBuffer))
{
    std::string ifn(filename);
    ifn.append(".md");
    b->read_from_file(ifn);
}

bool
MetaDataEncoder::metadata_decompress(std::vector<read_t>& reads, char *filename)
{
    //std::cout << " [SEQUENCE ID/METADATA]\n";
    uint32_t magic = 0;
    while (magic != MAGIC_NUMBER)
        magic = b->read(32);
    uint32_t curr_entry = 0;
    // Decode fields and separators
    num_fields = b->read(8);
    uint32_t entries = b->read(24);
    //std::cout << "\n  - Entries: [" << entries << "]\n";
    num_sep = b->read(8);
    //std::cout << "  - Fields: [" << unsigned(num_fields) << "]\n  - Delimiters: [" << unsigned(num_sep) << "]\n";
    reads.resize(entries);
    //std::cout << "  - Decoding fields\n";
    decode_fields();
    total_entries += entries;
    //std::cout << "  - Decoding separators: ";
    decode_separators();
    //for (int i = 0; i < sep.size(); i++)
    //    std::cout << sep[i];
   // std::cout << "\n";
    //std::cout << "  - Decoding entries\n";
    //std::cout << "Reads size: " << reads.size() << std::endl;
    for (uint32_t i = 0; i < entries; i++)
    {
        //printf("[%lu/%lu] %s\n", i, entries, reads[curr_entry-1].meta_data);
        decode_entry(reads[curr_entry++]);
        //if (i % 1000 == 0)
        //    printf("[%lu/%lu] %s\n", i, entries, reads[curr_entry-1].meta_data);
    }
   // std::cout << "Done decode." << std::endl;
    b->read_pad();
   // std::cout << "Padded." << std::endl;
    return b->read_is_end();
}



void 
MetaDataEncoder::metadata_compress(std::vector<read_t>& reads, char *filename)
{
    // Init
    b->init();
    std::cout << " [SEQUENCE ID/METADATA]\n";
    uint32_t entries = reads.size();
    
    bool success = false;
    while (!success)
    {
        // Identify the different fields in the sequence identifiers
        if (fields.size() == 0)
        {
            metadata_analyze(reads, entries);
        }

        b->write(MAGIC_NUMBER, 32);
 
        // Compress sequence identifier fields metadata
        b->write(num_fields, 8);         // Number of fields
        b->write(entries, 24);              // Number of entries
        b->write(num_sep, 8);
        std::cout << "  - Encoding field headers\n";
        for (auto it = fields.begin(); it != fields.end(); it++)
            (*it)->encode_metadata();

        std::cout << "  - Encoding separators\n";
        // Compress separators
        encode_separators();  

        std::cout << "  - Compressing metadata entries\n";
        // Compress metadata entries
        std::shared_ptr<std::vector<bb_entry_t> > bb_entries[N_THREADS];

        success = true;
#pragma omp parallel for num_threads(N_THREADS)    
        for (int i = 0; i < N_THREADS; i++)
        {
            bb_entries[i] = std::make_shared<std::vector<bb_entry_t> >();
            int boffset = entries * i / N_THREADS;
            int eoffset = entries * (i+1) / N_THREADS;

            success &= compress_parallel(reads.begin() + boffset, reads.begin() + eoffset, fields, bb_entries[i]);
            //std::cout << "Compressed entries " << boffset << " to " << eoffset << "\n";
        }
        if (success)
            for (int i = 0; i < N_THREADS; i++)
                for (std::vector<bb_entry_t>::iterator it = bb_entries[i]->begin(); it != bb_entries[i]->end(); it++)
                    b->write(it->value, it->length);   
        else 
        {
            fields.clear();
            b->init();
        }
    }
    std::string ofilename(filename);
    ofilename.append(".md");
    b->write_to_file(ofilename);
    //std::cout << "  - Compressed size: " << b->size() << " bytes --> " << ofilename << "\n";
}
