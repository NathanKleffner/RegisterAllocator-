#include "Compiler.hpp"
using namespace Compiler;


void printHelp(){
    cout<<"This program includes a handwritten scanner and parser for ILOC."<<endl;
    cout<<endl;
    cout<<"-t [filename] \t";
    cout<<"Prints a list of tokens from the input file."<<endl;
    cout<<"-p [filename] \t";
    cout<<"Prints legal ILOC code based of of input file. Does not preserve whitespace or comments."<<endl;
    cout<<"-h      \t";
    cout<<"Prints help statement"<<endl;
    cout<<"[filename] \t";
    cout<<"Prints an intermediate representation (list of instructions) based off of input file"<<endl;
}
int main(int argc, char** argv)
{   

    fstream myfile;
    Scanner s; 
    Parser p;
    Allocator a;
    int ch;
    while ((ch = getopt(argc, argv, ":hk: ")) != -1)
    {
        switch (ch)
        {
           
            case 'h':{
                printHelp();
                return 0;
                break;
            }
            case 'k':{
                myfile.open(argv[3]);
                bool valid = s.scan(myfile);
                if(!valid) return 0;
                vector<struct token> v = s.v;
                valid = p.makeIRVec(v);
                if(!valid) {
                    cout <<"PARSE ERROR"<<endl;
                    return 0;
                }
                vector <struct instruction> program = a.computeLastUse(p.v);
                a.allocate(program, s.constToInt(optarg) );
                a.prettyPrintTable(program);
                return 0;
            }
            default:
                break;
        }
    }
    myfile.open(argv[1]);
    bool valid = s.scan(myfile);
    if(!valid) return 0;
    vector<struct token> v = s.v;
    valid = p.makeIRVec(v);
    if(!valid) {
        cout <<"PARSE ERROR"<<endl;
        return 0;
    }
    vector <struct instruction> program = a.computeLastUse(p.v);
    a.allocate(program, 4 );
    a.prettyPrintTable(program);
    return 0;
};