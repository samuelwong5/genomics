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
MetaDataEncoder::metadata_analyze(std::vector<read_t>& reads, int entries)
{   
    std::string metadata(reads[0].meta_data);
    metadata.erase(0,1);
    metadata_separators(metadata);    // List of separators in order  
    int num_fields = sep.size() + 1;
    std::cout << "  - Analyzing Fields: [" << num_fields << "]\n";
    
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
        for (int i = 0; i < num_fields; i++)
            values[i].insert(v[i]);     
    }
    
    for (auto it = values.begin(); it != values.end(); it++)
    {
        std::string str("1");
        if (it->size() == 1)
        {
            std::cout << "    - Constant Alphanumeric: " << *it->begin() << std::endl;
            fields.push_back(new ConstantAlphanumericFieldEncoder(b, *it->begin()));
        }
        else if (it->size() > 0)
        {
            int max = 0;
            int min = 1000000000;
            bool numeric = true;
            for (auto sit = it->begin(); sit != it->end(); sit++)
            {
                
                if (!numeric || !EncodeUtil::is_numeric(*sit))
                {
                    numeric = false;
                    max = EncodeUtil::ceil_log(max, 10);
                    max = max > (int) sit->length() ? max : sit->length();
                }
                else
                { 
                    int val = atoi(sit->c_str());
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
                    fields.push_back(new NumericFieldEncoder(b, EncodeUtil::ceil_log(max + 1, 2), false));
                }
            }        
            else 
            {
                std::cout << "    - Alphanumeric  (Values: " << it->size() << ")\n";
                int bits_per_char = 8;
                if (true || it->size() * 10 < entries) // Enable mapping
                {
                    fields.push_back(new AlphanumericFieldEncoder(b, it->size(), true, *it));
                }
                else 
                {
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
    // Number of separators = num_fields - 1
    sep.clear();
    sep.push_back('@');
    for (int i = 1; i < num_fields; i++) 
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
    *md = '\0';
    //printf("[%d] %s\n", strlen(read.meta_data), read.meta_data);
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
    
    uint32_t curr_entry = 0;
    // Decode fields and separators
    num_fields = b->read(8);
    uint32_t entries = b->read(24);
    reads.resize(entries);
    decode_fields();
     total_entries += entries;
    decode_separators();
    for (uint32_t i = 0; i < entries; i++)
    {
        decode_entry(reads[curr_entry++]);
    }
    b->read_pad();
    return b->read_is_end();
}



void 
MetaDataEncoder::metadata_compress(std::vector<read_t>& reads, char *filename)
{
    // Init
    b->init();
    std::cout << " [SEQUENCE ID/METADATA]\n";
    int entries = reads.size();
    
    bool success = false;
    while (!success)
    {
        // Identify the different fields in the sequence identifiers
        if (fields.size() == 0)
        {
            metadata_analyze(reads, entries);
        }
 
        // Compress sequence identifier fields metadata
        b->write(fields.size(), 8);         // Number of fields
        b->write(entries, 24);              // Number of entries
        for (auto it = fields.begin(); it != fields.end(); it++)
            (*it)->encode_metadata();

        // Compress separators
        encode_separators();  

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
