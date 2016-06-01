#include "metadataencoder.hpp"


void
MetaDataEncoder::metadata_separators(std::string metadata)
{
    for (std::string::iterator it = metadata.begin(); it != metadata.end(); ++it)
    {
        if (*it == ' ' || *it == '.' || *it == '-' || *it == ':' || *it == '#')
            sep.push_back(*it);
    }
}


void 
MetaDataEncoder::split(std::string& str, std::vector<std::string>& parts) {
  size_t start, end = 0;
  static std::string delim = (" #.-:");
  while (end < str.size()) {
    start = end;
    while (start < str.size() && (delim.find(str[start]) != std::string::npos)) {
      start++;
    }
    end = start;
    while (end < str.size() && (delim.find(str[end]) == std::string::npos)) {
      end++;
    }
    if (end-start != 0) {
      parts.push_back(std::string(str, start, end-start));
    }
  }
}


void
MetaDataEncoder::metadata_analyze(std::vector<read_t>& reads, int entries)
{   
    std::string metadata(reads[0].meta_data);
    metadata.erase(0,1);
    metadata_separators(metadata);    // List of Separators in order  
    int num_fields = sep.size() + 1;
    std::cout << "  - Analyzing Fields: [" << num_fields << "]\n";
    
    // Initialize set of values for each field to count the number of 
    // total values
    std::vector<std::set<std::string> > values;
    for (int i = 0; i < num_fields; i++)
        values.push_back(std::set<std::string>());
    {
        std::vector<std::string> v;
        split(metadata, v);
        for (int i = 0; i < num_fields; i++)
            values[i].insert(v[i]);
    }
    // b->write(num_fields, 8);

    for (auto it = ++reads.begin(); it != reads.end(); it++)
    {
        std::vector<std::string> v;
        metadata = std::string(it->meta_data);
        metadata.erase(0,1);
        split(metadata, v);
        for (int i = 0; i < num_fields; i++)
            values[i].insert(v[i]);     
    }
    
   // b->write(entries, 24);
    std::cout << "  - Entries: " << entries << "\n";
    
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
            bool numeric = true;
            for (auto sit = it->begin(); sit != it->end(); sit++)
            {
                if (!numeric || !EncodeUtil::is_numeric(*sit))
                {
                    numeric = false;
                    max = EncodeUtil::ceil_log(max, 10);
                    max = max > sit->length() ? max : sit->length();
                }
                else
                {
                    int val = atoi(sit->c_str());
                    max = max > val ? max : val;                     
                }
            }
            if (numeric)
            {
                if (max == entries)
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
    for (int i = 1; i < num_fields; i++) 
        sep.push_back(smap[b->read(3)]);
}


void 
MetaDataEncoder::metadata_compress(std::vector<read_t>& reads, char *filename)
{
    b->init();
    std::cout << "[ STARTING COMPRESSION ]\n";
    // Encode sequence identifiers metadata
    
    int entries = reads.size();
    static bool analyzed = false;
    if (!analyzed)
    {
        metadata_analyze(reads, entries);
        analyzed = true;
    }

    b->write(fields.size(), 8);
    b->write(entries, 24);

    for (auto it = fields.begin(); it != fields.end(); it++)
        (*it)->encode_metadata();

    encode_separators();      
    int num_fields = fields.size();
    
    // Compress metadata entries
    std::string metadata;
    for (int rem = 0; rem < entries; rem++)
    {
        if (entries >= 100 && rem % (entries / 100) == 0)
            printf("\r  - Compressing [%3d%]", rem * 100 / entries);
        metadata = std::string(reads[rem].meta_data);
        //std::cout << "\n" << metadata;
        metadata.erase(0,1);
        std::vector<std::string> v;
        split(metadata, v);       
        for (int i = 0; i < num_fields; i++)
            fields[i]->encode(v[i]);  
    }
    std::string ofilename(filename);
    ofilename.append(".md");
    b->write_to_file(ofilename);
    std::cout << "\r  - Compressing [100%]\n  - Compressed size: " << b->size() << " bytes\n";
    
    // Cleanup
    for (auto it = fields.begin(); it != fields.end(); it++)
    {
        delete *it;
    }

    std::cout << "[ FINISHED COMPRESSION ]\n";
}

/*
void
MetaDataEncoder::decode(std::string ifilename, std::string ofilename = std::string(), int index = 1, int len = -1)
{ 
    std::cout << "[ STARTING DECOMPRESSION ]\n";
    std::shared_ptr<BitBuffer> b(new BitBuffer);
    b->read_from_file(ifilename);
    int num_fields = b->read(8);
    int entries = b->read(24);
    std::cout << " - Total Entries: " << entries << "\n";
    if (len > 0)
    {
        int end_index = entries < index + len ? entries : index + len;
        std::cout << " - Decompressing entries " << index << " to " << end_index << "\n";
        entries = entries < index + len ? entries - index : len;
    }
    std::vector<MetadataFieldEncoder*> fields;
    for (int i = 0; i < num_fields; i++)
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
                enc = new AutoIncrementingFieldEncoder(b, index - 1);
                enc->decode_metadata();
                fields.push_back(enc);
                break;
            }
        }
    }
    std::vector<char> sep;
    decode_separators(sep, b, num_fields - 1);
    sep.push_back('\n');
    std::ofstream of;
    std::streambuf *buf;
    if (ofilename.empty())
    {
        buf = std::cout.rdbuf();
    }
    else
    {
        of.open(ofilename);
        buf = of.rdbuf();
    }
    std::ostream os(buf);
    uint32_t entry_width = 0;
    for (int i = 0; i < num_fields; i++)
        entry_width += fields[i]->get_width();
    b->read_seek(entry_width * (index - 1)); 
    while (entries --> 0)
    {
        os << "@";
        for (int i = 0; i < num_fields; i++)
        {
            fields[i]->decode(os);
            os << sep[i];
        }
    }
    std::cout << "[ FINISHED DECOMPRESSION ]\n";
}


void 
encode(std::string ifilename, std::string ofilename, int entries = 1)
{
    std::cout << "[ STARTING COMPRESSION ]\n";
    // Encode sequence identifiers metadata
    std::shared_ptr<BitBuffer> b(new BitBuffer);
    std::vector<MetadataFieldEncoder*> fields;
    std::vector<char> sep;
    std::ifstream file(ifilename);
    //metadata_analyze(file, sep, fields, entries, b);
    
    for (auto it = fields.begin(); it != fields.end(); it++)
        (*it)->encode_metadata();

    encode_separators(sep, b);

    file.clear();
    file.seekg(0, std::ios::beg);

    // Compress sequence identifiers!       
    int num_fields = fields.size();
    std::string metadata;
    for (int rem = 0; rem < entries && std::getline(file, metadata); rem++)
    {
        if (entries >= 100 && rem % (entries / 100) == 0)
            printf("\r  - Compressing [%3d%]", rem * 100 / entries);
        metadata.erase(0,1);
        std::vector<std::string> v;
        split(metadata, v);       
        for (int i = 0; i < num_fields; i++)
            fields[i]->encode(v[i]);
        std::getline(file, metadata);
        std::getline(file, metadata);
        std::getline(file, metadata);      
    }
    b->write_to_file(ofilename);
    std::cout << "\r  - Compressing [100%]\n  - Compressed size: " << b->size() << " bytes\n";
    
    // Cleanup
    for (auto it = fields.begin(); it != fields.end(); it++)
    {
        delete *it;
    }
    file.close();
    std::cout << "[ FINISHED COMPRESSION ]\n";
}

int main(int argc, char** argv)
{
    if (argc < 3)
    {
        std::cout << "Usage:\n  metadataencoder --encode [FILE] [entries]\n metadataencoder --decode [FILE] [start] [length]" << std::endl;
        return -1;
    }
    int rows = argc >= 4 ? atoi(argv[3]) : 1000;
    std::string ifn(argv[2]);
    std::string ofn(ifn);
    if (strncmp(argv[1], "--encode", 8) == 0)
    {
        ofn.append(".seqid.cmprs");
        encode(ifn, ofn, rows);
    } 
    else if (strncmp(argv[1], "--decode", 8) == 0)
    {  
        ofn.erase(ofn.end()-5, ofn.end());
        if (argc >= 5)
            decode(ifn, ofn, atoi(argv[3]), atoi(argv[4]));
        else
            decode(ifn, ofn);
    } 
}*/
