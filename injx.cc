#include <cstdlib>
#include <cstdio>
#include <cstdint>
#include <cstring>

#include <stdexcept>
#include <iostream>
#include <fstream>
#include <filesystem>

#include <getopt.h>

#define INSTR_LEN   5
#define BASE_10     10
#define BASE_16     16

#define JMPREL32    0xe9
    
#ifdef DEBUG
#define PRINT_DBG(message)          \
    do {                            \
        cout << (message) << endl;  \
    } while (0)                     
#else
#define PRINT_DBG(message)  
#endif

using namespace std;

static bool is_hex_str(const char *str);
static string to_str_maybe_null(const char *str);
static void print_err(const string &message);
static void print_usage(void);
static void print_help(void); 

static struct option long_options[] = 
{
    {"target", required_argument, NULL, 't'},
    {"offset", required_argument, NULL, 'o'},
    {"inject", required_argument, NULL, 'i'},
    {"start" , required_argument, NULL, 's'},
    {"end"   , required_argument, NULL, 'e'},
    {"copy"  , required_argument, NULL, 'c'},
    {"jump"  , required_argument, NULL, 'j'},
    {"help"  , no_argument      , NULL, 'h'},
    { NULL   , no_argument      , NULL,  0 },
};

int main(int argc, char **argv)
{
    int      status                 = EXIT_FAILURE;
    uint8_t  instr[INSTR_LEN]       = {0};
    bool     help_flag              = false;
    char     *target_filename       = NULL, 
             *inject_filename       = NULL, 
             *copy_filename         = NULL, 
             *modify_filename       = NULL, 
             *buffer                = NULL, 
             ch                     = '\0';
    int32_t  inject_offset_start    = 0, 
             inject_offset_end      = -1, 
             modify_inject_offset   = -1, 
             modify_jmp_offset      = -1, 
             jaddr                  = 0, 
             inject_size            = -1, 
             target_filesize        = 0, 
             inject_filesize        = 0;
    
    fstream target, inject, copy, modify;
    
    while ((ch = getopt_long(argc, argv, "t:o:i:s:e:c:j:h", long_options, NULL)) != EOF)
    {
        switch (ch)
        {
            case 't':
            {
                target_filename = strdup(optarg);
                break;
            }
            case 'o':
            {
                try
                {
                    modify_inject_offset = stoi(optarg, NULL, is_hex_str(optarg) ? BASE_16 : BASE_10);
                }
                catch (invalid_argument &e)
                {
                    print_err("Invalid target file injection offset");
                    goto Error;
                }
                catch (out_of_range &e)
                {
                    print_err("Invalid target file injection offset");
                    goto Error;
                }
                break;
            }
            case 'i':
            {
                inject_filename = strdup(optarg);
                break;
            }
            case 's':
            {
                try
                {
                    inject_offset_start = stoi(optarg, NULL, is_hex_str(optarg) ? BASE_16 : BASE_10);
                }
                catch (invalid_argument &e)
                {
                    print_err("Invalid inject file start offset");
                    goto Error;
                }
                catch (out_of_range &e)
                {
                    print_err("Invalid inject file start offset");
                    goto Error;
                }
                break;
            }
            case 'e':
            {
                try
                {
                    inject_offset_end = stoi(optarg, NULL, is_hex_str(optarg) ? BASE_16 : BASE_10);
                }
                catch (invalid_argument &e)
                {
                    print_err("Invalid infect file end offset");
                    goto Error;
                }
                catch (out_of_range &e)
                {
                    print_err("Invalid infect file end offset");
                    goto Error;
                }
                break;
            }
            case 'c':
            {
                copy_filename = strdup(optarg);
                break;
            }
            case 'j':
            {
                try
                {
                    modify_jmp_offset = stoi(optarg, NULL, is_hex_str(optarg) ? BASE_16 : BASE_10);
                }
                catch (invalid_argument &e)
                {
                    print_err("Invalid target file jump offset");
                    goto Error;
                }
                catch (out_of_range &e)
                {
                    print_err("Invalid target file jump offset");
                    goto Error;
                }
                break;
            }
            case 'h':
            {
                help_flag = true;
                break;
            }
            default:
            {
                print_usage();
                goto Error;
            }
        }
    }
    
    if (help_flag)
    {
        print_help();
        goto Success;
    }
    
    if (target_filename == NULL)
    {
        print_err("Target file is a required parameter");
        goto Error;
    }
    
    if (inject_filename == NULL)
    {
        print_err("Injection file is a required parameter");
        goto Error;
    }
    
    if (modify_inject_offset < 0)
    {
        print_err("Target injection offset is a required parameter");
        goto Error;
    }
    
    try 
    {
        inject_filesize = filesystem::file_size(inject_filename);
    } 
    catch (filesystem::filesystem_error &e)
    {
        print_err("Opening file \"" + to_str_maybe_null(inject_filename) + "\"");
        goto Error;
    }
    if (inject_offset_start > inject_filesize || inject_offset_end > inject_filesize)
    {
        print_err("Start/end injection offsets too long");
        goto Error;
    }
    
    if (inject_offset_end < 0)
    {
        inject_offset_end = inject_filesize;
    }
    
    inject_size = inject_offset_end + 1 - inject_offset_start;
    if (inject_size < 0 || inject_size > inject_filesize)
    {
        print_err("Start/end injection offsets invalid");
        goto Error;
    }
     
    if (copy_filename != NULL)
    {
        target.open(target_filename, ios::in | ios::binary);
        if (!target)
        {
            print_err("Opening file \"" + to_str_maybe_null(target_filename) + "\"");
            goto Error;
        }
        
        copy.open(copy_filename, ios::out | ios::binary);
        if (!copy)
        {
            print_err("Creating or opening file \"" + to_str_maybe_null(copy_filename) + "\"");
            goto Error;
        }
           
        copy << target.rdbuf();
        
        target.close();
        if (!target)
        {
            print_err("Closing file \"" + to_str_maybe_null(target_filename) + "\"");
            goto Error;
        }
        
        copy.close();
        if (!target)
        {
            print_err("Closing file \"" + to_str_maybe_null(copy_filename) + "\"");
            goto Error;
        }
        
        modify_filename = copy_filename;
    }
    else
    {
        modify_filename = target_filename;
    }
    
    inject.open(inject_filename, ios::in | ios::binary);
    if (!inject)
    {
        print_err("Opening file \"" + to_str_maybe_null(inject_filename) + "\"");
        goto Error;
    }
    
    inject.seekg(inject_offset_start);
    
    buffer = new char[inject_size];
    
    inject.read(buffer, inject_size);
    if (!inject)
    {
        print_err("Reading from file \"" + to_str_maybe_null(inject_filename) + "\"");
        goto Error;
    }
    
    inject.close();
    if (!inject)
    {
        print_err("Closing file \"" + to_str_maybe_null(inject_filename) + "\"");
        goto Error;
    }
    
    modify.open(modify_filename, ios::out | ios::in | ios::binary);
    if (!modify)
    {
        print_err("Opening file \"" + to_str_maybe_null(modify_filename) + "\"");
        goto Error;
    }

    modify.seekp(modify_inject_offset, ios::beg);
    
    modify.write(buffer, inject_size);
    if (!modify)
    {
        print_err("Writing to file \"" + to_str_maybe_null(modify_filename) + "\"");
        goto Error;
    }
    
    if (modify_jmp_offset >= 0)
    {
        try
        {
            target_filesize = filesystem::file_size(target_filename);
        }
        catch (filesystem::filesystem_error &e)
        {
            print_err("Opening file \"" + to_str_maybe_null(target_filename) + "\"");
            goto Error;
        }
        if (modify_jmp_offset > target_filesize)
        {
            print_err("Jump offset is larger than target file size");
            goto Error;
        }
    
        instr[0] = JMPREL32;
        jaddr = modify_inject_offset - modify_jmp_offset - sizeof(instr);
        memcpy(&instr[1], &jaddr, sizeof(jaddr));
        
        cout << "Instr: ";
        for (status = 0; status < (int) sizeof(instr); ++status)
        {
            cout << hex << instr[status] << " ";
        }
        cout << endl;
        
        modify.seekp(modify_jmp_offset, ios::beg);
        
        modify.write((char *) instr, sizeof(instr));
        if (!modify)
        {
            print_err("Writing to file \"" + to_str_maybe_null(modify_filename) + "\"");
            goto Error;
        }
    }
    
    modify.close();
    if (!modify)
    {
        print_err("Closing file \"" + to_str_maybe_null(modify_filename) + "\"");
        goto Error;
    }
   
Success:
    status = EXIT_SUCCESS;
    
Error:
    if (buffer          != NULL) delete [] buffer;
    if (target_filename != NULL) delete target_filename;
    if (inject_filename != NULL) delete inject_filename;
    if (copy_filename   != NULL) delete copy_filename;
    
    return status;
}

static bool is_hex_str(const char *str)
{
    if (strncmp(str, "0x", 2) == 0) return true;
    return false;
}

static string to_str_maybe_null(const char *str)
{
    return (str == NULL) ? string() : string(str);
}

static void print_err(const string &message)
{
    cerr << "[ERROR] " << message << endl;
}

static void print_usage(void)
{
    cout 
    << "Usage: injx *[-t|--target] <filename> *[-o|--offset] <offset> *[-i|--inject] <filename> " 
    << "[-s|--start] <offset> [-e|--end] <offset> [-c|--copy] <filename> [-j|--jump] <offset> [-h|--help]" 
    << endl;
}

static void print_help(void)
{
    print_usage();
    cout
    << "\n"
    << "-t | --target <filename>  [Req] File to inject into\n" 
    << "-o | --offset <offset>    [Req] File offset at which to inject in target file\n"
    << "-i | --inject <filename>  [Req] File from which to inject\n"
    << "-s | --start  <offset>    [Opt] File offset to start from inject file, else beginning of file\n"
    << "-e | --end    <offset>    [Opt] File offset to end from inject file, else end of file\n"
    << "-c | --copy   <filename>  [Opt] Copy target file into new file, else modify target file directly\n"
    << "-j | --jump   <offset>    [Opt] Set an x86 jump to the injected data at the given offset\n"
    << "-h | --help               [Opt] Print help about using injx, then exit"
    << endl;
}

