#include "Compiler.hpp"
using namespace Compiler;


void printHelp(){
    cout<<"Welcome project 1. This program includes a handwritten scanner and parser for ILOC."<<endl;
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
    while ((ch = getopt(argc, argv, "tph ")) != -1)
    {
        switch (ch)
        {
            case 't':{
                 myfile.open(argv[2]);
                bool valid = s.scan(myfile);
                if(!valid) return 0;
                s.prettyPrintVector();
                return 0;   
            }
            case 'p':{
                myfile.open(argv[2]);
                bool valid = s.scan(myfile);
                if(!valid) return 0;
                vector<struct token> v = s.v;
                valid = p.makeIRVec(v);
                if(!valid){
                    cout<<"PARSE ERROR"<<endl;
                    return 0;
                }
                p.notPrettyPrint();
                return 0;
            }
            case 'h':{
                printHelp();
                return 0;
                break;
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
    //p.prettyPrintTable();
    vector <struct instruction> program = a.computeLastUse(p.v);
    a.prettyPrintTable(program);
    return 0;
};