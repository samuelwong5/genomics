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


static void
analyze_parallel(std::vector<read_t>::iterator begin, std::vector<read_t>::iterator end, MetadataAnalysis *ma, uint32_t fields, uint32_t entries)
{  
    bool constant[fields];
    bool numeric[fields];
    uint32_t max[fields];
    uint32_t min[fields];
    std::string strvals[fields];

    std::string metadata((begin++)->meta_data);
    
    {
        std::vector<std::string> v;
        metadata.erase(0, 1);
        EncodeUtil::split(metadata, v);
        for (uint32_t i = 0; i < fields; i++)
        {
            constant[i] = true;
            numeric[i] = EncodeUtil::is_numeric(v[i]);
            if (numeric[i])
            {
                max[i] = atoi(v[i].c_str());
                min[i] = max[i];
            } else { max[i] = v[i].length(); }
            strvals[i] = v[i];
        }
    }

    for (auto it = begin; it != end; it++)
    {
        std::vector<std::string> v;
        metadata = std::string(it->meta_data);
        metadata.erase(0, 1);
        EncodeUtil::split(metadata, v);
        for (uint32_t i = 0; i < fields; i++)
        {
            if (constant[i] && strvals[i] != v[i])
            {
                // No longer constant alphanumeric;
                constant[i] = false;
            } 

            if (!constant[i])
            {
                // Prev values not numeric OR curr value not numeric
                if (!numeric[i] || !EncodeUtil::is_numeric(v[i]))
                {  
                    // Curr value not numeric
                    if (numeric[i] && !EncodeUtil::is_numeric(v[i]))
                        max[i] = EncodeUtil::ceil_log(max[i], 10);
                    numeric[i] = false;
                    max[i] = max[i] > v[i].length() ? max[i] : v[i].length();
                }
                else
                {
                    uint32_t val = atoi(v[i].c_str());
                    max[i] = max[i] > val ? max[i] : val;
                    min[i] = min[i] < val ? min[i] : val;
                }
            }

        }
    }

    for (uint32_t i = 0; i < fields; i++)
    {
        if (constant[i])
        {
            ma[i].type = MetadataFieldType::CONSTANT_ALPHANUMERIC;
            ma[i].val = strvals[i]; 
        }
        else if (numeric[i] && (max[i] - min[i] + 1 == entries))
            ma[i].type = MetadataFieldType::AUTOINCREMENTING;
        else if (numeric[i])
            ma[i].type = MetadataFieldType::NUMERIC;
        else
            ma[i].type = MetadataFieldType::ALPHANUMERIC;
        ma[i].width = max[i];
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
    
    MetadataAnalysis mas[N_THREADS][num_fields];
    
    const uint32_t read_size = reads.size();
#pragma omp parallel for num_threads(N_THREADS)    
    for (int i = 0; i < N_THREADS; i++)
    {
        int boffset = read_size * i / N_THREADS;
        int eoffset = read_size * (i + 1) / N_THREADS;
        analyze_parallel(reads.begin() + boffset, reads.begin() + eoffset, mas[i], num_fields, eoffset - boffset);
    }

    // Combine 
    for (uint32_t j = 0; j < num_fields; j++)
    {
        bool constant = true;
        std::string val = mas[0][j].val;
        MetadataFieldType mft = AUTOINCREMENTING;
        for (int i = 0; i < N_THREADS; i++)
        {
            if (mas[i][j].type != MetadataFieldType::CONSTANT_ALPHANUMERIC || val != mas[i][j].val)
                constant = false;
            if (mas[i][j].type == MetadataFieldType::NUMERIC && mft == MetadataFieldType::AUTOINCREMENTING)
                mft = MetadataFieldType::NUMERIC;
            else if (mas[i][j].type == MetadataFieldType::ALPHANUMERIC)
                mft = MetadataFieldType::ALPHANUMERIC;
            else if (mas[i][j].type == MetadataFieldType::CONSTANT_ALPHANUMERIC)
            {
                if (!EncodeUtil::is_numeric(mas[i][j].val))
                     mft = MetadataFieldType::ALPHANUMERIC;
                else if (mft == MetadataFieldType::AUTOINCREMENTING)
                     mft = MetadataFieldType::NUMERIC;
            }
        } 
  
        if (constant) 
        {
            fields.push_back(new ConstantAlphanumericFieldEncoder(b, val));
        }
        else if (mft == MetadataFieldType::AUTOINCREMENTING)
        {
            fields.push_back(new AutoIncrementingFieldEncoder(b, 1));
        }
         
        else if (mft == MetadataFieldType::NUMERIC)
        {
            uint32_t max = 0;
            for (int i = 0; i < N_THREADS; i++)
                max = max > mas[i][j].width ? max : mas[i][j].width;
            uint32_t wd = EncodeUtil::ceil_log(max + 1, 2);
            fields.push_back(new NumericFieldEncoder(b, wd, false));
        }
        else // ALPHANUMERIC
        {
            std::set<std::string> vals;
            for (auto it = reads.begin(); it != reads.end(); it++)
            {
                std::vector<std::string> v;
                std::string metadata(it->meta_data);
                metadata.erase(0, 1);
                EncodeUtil::split(metadata, v);
                vals.insert(v[j]);
            }

            int bits_per_char = 8;
            if (vals.size() * 100 < entries) // Enable mapping
            {
                fields.push_back(new AlphanumericFieldEncoder(b, vals.size(), true, vals));
            }
            else 
            {
                uint32_t w = 0;
                for (int i = 0; i < N_THREADS; i++)
                {
                    if (mas[i][j].type == MetadataFieldType::ALPHANUMERIC || mas[i][j].type == MetadataFieldType::CONSTANT_ALPHANUMERIC)
                        w = w > mas[i][j].width ? w : mas[i][j].width;
                    else
                    {
                        uint32_t wd = EncodeUtil::ceil_log(mas[i][j].width, 10);
                        w = w > wd ? w : wd;
                    }
                }
                fields.push_back(new AlphanumericFieldEncoder(b, w * bits_per_char, false, vals));
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
            }
            
        // Reanalyze metadata if failed
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
    uint32_t magic = 0;
    while (magic != MAGIC_NUMBER)
        magic = b->read(32);
    uint32_t curr_entry = 0;
    
    // Decode fields and separators
    num_fields = b->read(8);
    uint32_t entries = b->read(24);
    num_sep = b->read(8);
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
        b->write(entries, 24);           // Number of entries
        b->write(num_sep, 8);
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
    
    // Write to file
    std::string ofilename(filename);
    ofilename.append(".md");
    b->write_to_file(ofilename);
}
