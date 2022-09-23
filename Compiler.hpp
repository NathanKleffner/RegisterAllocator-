#include<fstream>
#include<iostream>
#include<string>
#include<ctype.h>
#include<vector>
#include<unistd.h>
#include<map>
#include<iomanip>
#include<assert.h>
using namespace std;

namespace Compiler  
{
    enum opcode {load,loadI,store,add,sub,mult,lshift,rshift,output,nop};
    inline string ToString(opcode o)
    {
        switch(o)
        {
            case load: return "load"; 
            case loadI: return "loadI";
            case store: return "store";
            case add: return "add";
            case sub: return "sub";
            case mult: return "mult";
            case lshift: return "lshift";
            case rshift: return "rshift";
            case output: return "output";
            case nop: return "nop";
        }
    }
    enum tokenCategory {INST,REG,CONST,COMMA,ARROW};
    struct token{
        tokenCategory cat;
        opcode value;
        int num;
    };

    struct op{
        int sr;
        int vr;
        int pr;
        int nu;
    };
    struct instruction{
        opcode op; 
        struct op OP1;
        struct op OP2;
        struct op OP3;
    };


    struct SRtoVR {
        int sr;
        int SRtoVR; 
        int lastUse;
        int mem;
    };

    class Allocator
    {
        public: 
            vector<struct SRtoVR> SRtoVRTable; 
            int vrName;
            void initializeSRtoVr(int size);
            vector<struct instruction>& computeLastUse(vector<struct instruction>& program);
            void update(struct op OP, int index);
            void prettyPrintTable(vector<struct instruction>& v);
    };

    class Parser
    {
        public:
            vector<struct instruction> v; 
            bool makeIRVec(vector<struct token>& v);
            void prettyPrintTable();
            void notPrettyPrint();
    };
    class Scanner 
    {

        public:
            vector<struct token> v;
            bool scan(fstream& in);
            void start(fstream& in);
            void prettyPrintVector();
            int constToInt(string s);
        private:
            void state1(fstream& in);
            void state2(fstream& in);
            void state3(fstream& in);
            void state4(fstream& in);
            void state5(fstream& in);
            void state6(fstream& in);
            void state7(fstream& in);
            void state8(fstream& in);
            void state9(fstream& in);
            void state10(fstream& in);
            void state11(fstream& in);
            void state12(fstream& in);
            void state13(fstream& in);
            void state14(fstream& in, char back);
            void state15(fstream& in, char back);
            void state16(fstream& in, char back);
            void state17(fstream& in, char back);
            void state18(fstream& in, char back);
            void state18Mult(fstream& in);
            void state19(fstream& in);
            void state20(fstream& in);
            void state21(fstream& in);
            void state22(fstream& in);
            void state23(fstream& in);
            void state24(fstream& in);
            void state25(fstream& in);
            void state26(fstream& in);
            void state27(fstream& in);
            void state28(fstream& in);
            void state29(fstream& in);
            void state30(fstream& in);
            void state31(fstream& in);
            void state32(fstream& in);
            void state33(fstream& in);
            void commentStart(fstream& in);
            void commentState1(fstream& in);
            void arrowStart(fstream& in);
            void arrowAccept(fstream& in);
            void registerState1(fstream& in,int n);
            void commaAccept(fstream& in);
            void constantState(fstream& in,int n);     
    };
}