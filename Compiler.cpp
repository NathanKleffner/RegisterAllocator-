#include "Compiler.hpp"
using namespace Compiler;

//Helper function for prettyPrintTable
string printIfValid(int i){
    return i != -1 ? to_string(i) : "";
}

void Allocator::prettyPrintTable(vector<struct instruction>& v)
{

    cout << "________________________________________________________________________________________________________________________\n"
         << "|INDEX  |OPCODE |              OP1              |              OP2              |              OP3              |  NEXT |\n"
         << "|       |       |  SR   |  VR   |  PR   |  NU   |  SR   |  VR   |  PR   |  NU   |  SR   |  VR   |  PR   |  NU   |  OP   |\n"
         << "|_______________________________________________________________________________________________________________|_______|\n";
    for (int i =0; i < v.size(); i++){
        instruction inst = v[i];
        cout << "|" << i << "\t|" 
             << ToString(inst.op) << "\t|"
             << printIfValid(inst.OP1.sr) << "\t|"
             << printIfValid(inst.OP1.vr) << "\t|"
             << printIfValid(inst.OP1.pr) << "\t|"
             << printIfValid(inst.OP1.nu) << "\t|"
             << printIfValid(inst.OP2.sr) << "\t|"
             << printIfValid(inst.OP2.vr) << "\t|"
             << printIfValid(inst.OP2.pr) << "\t|"
             << printIfValid(inst.OP2.nu) << "\t|"
             << printIfValid(inst.OP3.sr) << "\t|"
             << printIfValid(inst.OP3.vr) << "\t|"
             << printIfValid(inst.OP3.pr) << "\t|"
             << printIfValid(inst.OP3.nu) << "\t|"
             << i+1 << "\t|" << '\n';
    }    
}

void Allocator::initializeSRtoVR(int size)
{
    for(int i = 0; i <= size;i++)
    {
        struct SRtoVR temp;
        temp.sr = i;
        temp.SRtoVR = -1;
        temp.lastUse = size;
        SRtoVRTable.push_back(temp);
    }
}

void Allocator::initializeVRtoPR(int size)
{
    for(int i = 0; i <= size; i++)
    {
        struct VRtoPR temp;
        temp.vr = i;
        temp.VRtoPR = -1;
        temp.mem = -1;
        temp.nextUse = -1;
        VRtoPRTable.push_back(temp);
    }
}

//OPSR corresponds to the OP source register (source1, source2, destination)
void Allocator::update(struct op &OP, int index)
{
    if(SRtoVRTable[OP.sr].SRtoVR == -1)
    {
        SRtoVRTable[OP.sr].SRtoVR = vrName++;
    }
    OP.vr = SRtoVRTable[OP.sr].SRtoVR;
    OP.nu = SRtoVRTable[OP.sr].lastUse;
    SRtoVRTable[OP.sr].lastUse = index;
    return;
}


vector<struct instruction>& Allocator::computeLastUse(vector<struct instruction>& program)
{
    initializeSRtoVR(program.size());
    vrName = 0;
    for(int i = program.size()-1; i >= 0; i--)
    {
        struct instruction &x = program[i];
        if(x.OP3.sr != -1)
        {
            update(x.OP3,i);
            SRtoVRTable[x.OP3.sr].SRtoVR = -1;
            SRtoVRTable[x.OP3.sr].lastUse = program.size();
        }
        if (x.OP1.sr != -1 && x.op != output && x.op != loadI) update(x.OP1,i);
        if (x.OP2.sr != -1) update(x.OP2,i);
    }

    return program;
}

void Allocator::assignPR(vector<struct instruction>& program, int opnum, int& index){
    
    op OP = opnum == 1 ? program[index].OP1 : (opnum == 2 ? program[index].OP2 : (program[index].OP3));
    if (OP.vr == -1)
        return;
    VRtoPRTable[OP.vr].nextUse = OP.nu;// not sure when to do this

    // If this VR doesn't have a PR
    if(VRtoPRTable[OP.vr].VRtoPR == -1){
            cout << "\tno pr for vr " << OP.vr << '\n';
            // If the stack is empty, spill
            if (prStack.empty()){
                int spillPR = -1;
                int spillVR = -1;
                int max = -1;
                // spill the pr that has the max next use 
                for (int i = 0; i < VRtoPRTable.size(); i++){
                    cout << "\t\tvr " << i << " pr " << VRtoPRTable[i].VRtoPR << " nu " << VRtoPRTable[i].nextUse << '\n';
                    if (VRtoPRTable[i].VRtoPR != -1 && VRtoPRTable[i].nextUse > max){
                        spillVR = i;
                        spillPR = VRtoPRTable[i].VRtoPR;
                        max = VRtoPRTable[i].nextUse;
                    }
                }
                cout << "\tspill vr " << spillVR << " pr " << spillPR << '\n'; 

                // loadI ? => r0
                instruction loadIInst = {
                    loadI, 
                    {1024,-1,-1,-1},
                    {-1,-1,-1,-1},
                    {-1,-1,0,-1},
                };
                // store r? => r0
                instruction storeInst = {
                    store, 
                    {-1,-1,spillPR,-1},
                    {-1,-1,-1,-1},
                    {-1,-1,0,-1},
                };
                
                program.emplace(program.begin() + index++, loadIInst);
                program.emplace(program.begin() + index++, storeInst);
                OP = opnum == 1 ? program[index].OP1 : (opnum == 2 ? program[index].OP2 : (program[index].OP3));

                prStack.push_back(spillPR);
                VRtoPRTable[spillVR].mem = 1024;
                VRtoPRTable[spillVR].VRtoPR = -1;

            }

            // Assign the PR
            int freePR = prStack.back();
            prStack.pop_back();
            VRtoPRTable[OP.vr].VRtoPR = freePR;
            cout << "\tassign vr " << OP.vr << " pr " << freePR << '\n';
            // Load op.VR into freePR????

            // If this VR has been spilled, unspill it
            if (VRtoPRTable[OP.vr].mem != -1){
                cout << "\tunspill vr " << OP.vr << " from mem " << VRtoPRTable[OP.vr].mem << '\n';
                instruction loadIInst = {
                    loadI, 
                    {VRtoPRTable[OP.vr].mem,-1,-1,-1},
                    {-1,-1,-1,-1},
                    {-1,-1,0,-1},
                };
                instruction loadInst = {
                    load, 
                    {-1,-1,0,-1},
                    {-1,-1,-1,-1},
                    {-1,-1,freePR,-1},
                };

                program.emplace(program.begin() + index++, loadIInst);
                program.emplace(program.begin() + index++, loadInst);
                OP = opnum == 1 ? program[index].OP1 : (opnum == 2 ? program[index].OP2 : (program[index].OP3));

            }
    }

    cout << " set vr " << OP.vr << " pr " << VRtoPRTable[OP.vr].VRtoPR << '\n';
    op& OPref = opnum == 1 ? program[index].OP1 : (opnum == 2 ? program[index].OP2 : (program[index].OP3));
    OPref.pr = VRtoPRTable[OP.vr].VRtoPR;
}

vector<struct instruction>& Allocator::allocate(vector<struct instruction>& program)
{
    initializeVRtoPR(3);
    for (int i = 2; i >= 1; i--)
        prStack.push_back(i);

    int infinity = program.size();
    
    int i = 0;
    while (i < program.size())
    {
        cout << "instruction " << i << ": " << ToString(program[i].op) << '\n';
        struct instruction &x = program[i];
        cout << "OP1\n";
        if (x.op != loadI && x.op != output)
            assignPR(program, 1, i);
        cout << "OP2\n";
        assignPR(program, 2, i);

        // If never used again, free it
        if (x.OP1.nu == infinity){
            cout << "\tfree op1 pr " << x.OP1.pr << '\n';
            prStack.push_back(x.OP1.pr);
            VRtoPRTable[x.OP1.vr].VRtoPR = -1;
        }
        if (x.OP2.nu == infinity){
            cout << "\tfree op2 pr " << x.OP1.pr << '\n';
            prStack.push_back(x.OP2.pr);
            VRtoPRTable[x.OP2.vr].VRtoPR = -1;
        }
        
        cout << "OP3\n";
        assignPR(program, 3, i);
        i++;
    }

    return program;
}

void Parser::prettyPrintTable()
{

    cout<<"|"<<"index"<<setw(4)<<"|"<<"opcode"<<setw(4)<<"||"<<"op1"<<setw(4)<<"|"<<"op2"<<setw(5)<<"|"<<"dest"<<setw(4)<<"|"<<endl;
    cout<<"|"<<setw(9)<<"|"<<setw(10)<<"||"<<"sr"<<setw(5)<<"|"<<"sr"<<setw(6)<<"|"<<"sr"<<setw(6)<<"|"<<endl;
    
    for(int i = 0; i < v.size(); i++)
    {
        struct instruction x = v[i]; 
        string opc = ToString(x.op);
        cout<<"|"<<i<<setw(9-to_string(i).length())<<"|";
        cout<<opc<<setw(10-opc.length())<<"||";
        if(x.op == 9) cout<<"  -   "<<"|";
        else if (x.op ==1||x.op==8) cout<<x.OP1.sr<<setw(7-to_string(x.OP1.sr).length())<<"|";
        else cout<<"r"<<x.OP1.sr<<setw(6-to_string(x.OP1.sr).length())<<"|";
        if(x.OP2.sr == -1)cout<<"   -   "<<"|";
        else cout<<"r"<<x.OP2.sr<<setw(7-to_string(x.OP2.sr).length())<<"|";
        if(x.OP3.sr == -1)cout<<"   -   "<<"|";
        else cout<<"r"<<x.OP3.sr<<setw(7-to_string(x.OP3.sr).length())<<"|";
        cout<<endl; 
    
    }
}

void Parser::notPrettyPrint()
{
    for(struct instruction i: v)
    {
        int padding;
        switch(i.op)
        {
            case 0: {
                padding = 9 - to_string(i.OP1.sr).length();
                cout <<"load"<<setw(6)<<"r"<<i.OP1.sr<<setw(padding)<<"=>"<<setw(4)<<"r"<<i.OP3.sr<<endl;
                break;
            }
            case 1:{
                padding = 10 - to_string(i.OP1.sr).length();
                cout <<"loadI    "<<i.OP1.sr<<setw(padding)<<"=>"<<setw(4)<<"r"<<i.OP3.sr<<endl;
                break;
            }
            case 2:{
                padding = 9 - to_string(i.OP1.sr).length();
                cout <<"store"<<setw(5)<<"r"<<i.OP1.sr<<setw(padding)<<"=>"<<setw(4)<<"r"<<i.OP2.sr<<endl;
                break;
            }
            case 3: case 4: case 5: case 6: case 7: {
                padding = 7 - (to_string(i.OP1.sr).length()+to_string(i.OP2.sr).length());
                cout <<ToString(i.op)<<setw(10-ToString(i.op).length())<<"r"<<i.OP1.sr<<","<<"r"<<i.OP2.sr<<setw(padding)<<"=>"<<setw(4)<<"r"<<i.OP3.sr<<endl;
                break;
            }
            case 8:{
                cout<<"output   "<<i.OP1.sr<<endl;
                break;
            }
            case 9:{
                cout<<"nop"<<endl;
                break;
            }
        }
    }
}

bool Parser::makeIRVec(vector<struct token>& ts)
{
    int i = 0; 
    bool valid = true;
    while(i < ts.size() && valid)
    {
        token t = ts[i];
        if(t.cat != INST)
        {
            valid = false;
            return valid;
        }
        instruction inst;
        inst.op = t.value;
        inst.OP1.vr = -1;
        inst.OP2.vr = -1;
        inst.OP3.vr = -1;
        inst.OP1.pr = -1;
        inst.OP2.pr = -1;
        inst.OP3.pr = -1;
        inst.OP1.nu = -1;
        inst.OP2.nu = -1;
        inst.OP3.nu = -1;
        switch(t.value)
        {
            case 0: {//load
                if(ts[i+1].cat != REG) valid = false;
                inst.OP1.sr = ts[i+1].num;
                if(ts[i+2].cat != ARROW) valid = false;
                inst.OP2.sr = -1;
                if(ts[i+3].cat != REG) valid = false;
                inst.OP3.sr = ts[i+3].num;
                i+= 4;
                break;
            }
            case 1:{ //loadI
                if(ts[i+1].cat != CONST) valid = false;
                inst.OP1.sr = ts[i+1].num;
                if(ts[i+2].cat != ARROW) valid = false;
                inst.OP2.sr= -1;
                if(ts[i+3].cat != REG)valid = false;
                inst.OP3.sr = ts[i+3].num;
                i+= 4;
                break;
            } 
            case 2: { //store
                if(ts[i+1].cat != REG) valid = false;
                inst.OP1.sr = ts[i+1].num;
                if(ts[i+2].cat != ARROW) valid = false;
                if(ts[i+3].cat != REG) valid = false;
                inst.OP2.sr = ts[i+3].num;
                inst.OP3.sr = -1;
                i+=4;
                break;
            }
            case 3: case 4: case 5: case 6: case 7: { //arithop r1,r2 => r3
                if(ts[i+1].cat != REG) valid = false; 
                inst.OP1.sr = ts[i+1].num;
                if(ts[i+2].cat != COMMA) valid = false;
                if(ts[i+3].cat != REG) valid = false;
                inst.OP2.sr = ts[i+3].num;
                if(ts[i+4].cat != ARROW) valid = false;
                if(ts[i+5].cat != REG) valid = false;
                inst.OP3.sr = ts[i+5].num;
                i+=6; 
                break;
            }
            case 8: {//output
                if(ts[i+1].cat != CONST) valid = false;
                inst.OP1.sr = ts[i+1].num;
                inst.OP2.sr = -1;
                inst.OP3.sr = -1;
                i += 2;
                break;
            }
            case 9: {//nop
                inst.OP1.sr = -1;
                inst.OP2.sr = -1;
                inst.OP3.sr = -1;
                i+=1;
                break;
            }
        }
        v.push_back(inst);
    }
    return valid;
}

void Scanner::prettyPrintVector()
{
    for (struct token t: v)
    {   
        cout <<"<";
        switch(t.cat)
        {
            case INST: cout<<"INST, "+ToString(t.value); break;
            case ARROW: cout<<"ARROW, \'=>\'"; break;
            case COMMA: cout <<"COMMA, \',\'"; break;
            case REG: cout<<"REG, "<<'r'<<t.num; break;
            case CONST: cout <<"CONST, "<<t.num; break;
        }
        cout<<">"<<endl;
    }
}

int Scanner::constToInt(string s)
{
    int n= 0;
    for(int i =1;i < s.length();i++)
    {
        char c = s[i];
        n = n*10 + (c - '0');
    }
    return n;
}

bool Scanner::scan(fstream& in)
{
    while(in.peek() != EOF)
    {
        try {
            start(in);
        } catch (const std::invalid_argument& e)
        {
            cout <<e.what()<<endl;
            return false;
        }
    }
    return true;
}
void Scanner::start(fstream& in)
{
    char c;
    in.get(c);
    if(isdigit(c))
    {
        int n = (c - '0');
        constantState(in, n);
    }
    else
    {
        switch(c){
            case 's': 
                return state1(in);
                break;
            case 'l':
                return state8(in);
                break;
            case 'r':
                return state13(in);
                break;
            case 'm':
                return state19(in);
                break;
            case 'a':
                return state22(in);
                break;
            case 'n':
                return state25(in);
                break;
            case 'o':
                return state28(in);
                break;
            case ' ':
                return start(in);
                break;
            case '=':
                return arrowStart(in);
                break;
            case ',':
                return commaAccept(in);
                break;
            case '\n':
                return start(in);
                break;
            case '\t':
                return start(in);
                break;
            case '/':
                return commentStart(in);
                break;
            case EOF: case '\0': case '\1':
                return;
                break;
            default:{
                return;
                break;
            }
        }
    }
}

void Scanner::commaAccept(fstream& in)
{
    struct token t;
    t.cat = COMMA;
    v.push_back(t);
    return;
}

void Scanner::registerState1(fstream& in, int n)
{
    char c;
    in.get(c);
    if(isdigit(c)){
        n = n*10 + (c - '0');
        registerState1(in,n);
    } else {
        in.unget(); //cursed
        struct token t;
        t.cat = REG;
        t.num = n;
        v.push_back(t);
        return;
    }
}


void Scanner::constantState(fstream& in,int n)
{
    char c;
    in.get(c);
    if(isdigit(c)){
        n = n*10 + (c - '0');
        constantState(in,n);
    }else {
        in.unget(); //cursed
        struct token t;
        t.cat = CONST;
        t.num = n;
        v.push_back(t);
        return;
    }
}

void Scanner::arrowStart(fstream& in)
{
    char c;
    in.get(c);
    switch(c){
        case '>':
            arrowAccept(in);
            break;
        default:
            cout<<"INVALID CHARACTER = "<<c<<endl;
            throw invalid_argument("ARROW ERROR");
            break;
    }
}
void Scanner::arrowAccept(fstream& in)
{
    struct token t;
    t.cat = ARROW;
    v.push_back(t);
     return;
}
void Scanner::commentStart(fstream& in)
{
    char c;
    in.get(c);
    switch(c){
        case '/':
            commentState1(in);
            break;
        default:
            cout<<"INVALID CHARACTER = "<<c<<endl;
            throw std::invalid_argument("COMMENT ERROR");
            break;
    }
}
void Scanner::commentState1(fstream& in)
{
    char c;
    in.get(c);
    switch(c){
        case '\n':
             return;
            break;
        default:
            commentState1(in);
            break;
    }
}
void Scanner::state1(fstream& in)
{
    char c;
    in.get(c);
    switch(c){
        case 't':
            state2(in);
            break;
        case 'u':
            state6(in);
            break;
        default:
            cout<<"INVALID CHARACTER = "<<c<<endl;
            throw std::invalid_argument("ERROR STATE1");
            break;
    }
}

void Scanner::state2(fstream& in)
{
    char c;
    in.get(c);
    switch(c){
        case 'o':
            state3(in);
            break;
        default:
            cout<<"INVALID CHARACTER = "<<c<<endl;
            throw std::invalid_argument("ERROR STATE2");
            break;
    }
}

void Scanner::state3(fstream& in)
{
    char c;
    in.get(c);
    switch(c){
        case 'r':
            state4(in);
            break;
        default:
            cout<<"INVALID CHARACTER = "<<c<<endl;
            throw std::invalid_argument("ERROR STATE3");
            break;
    }
}

void Scanner::state4(fstream& in)
{
    char c;
    in.get(c);
    switch(c){
        case 'e':
            state5(in);
            break;
        default:
            cout<<"INVALID CHARACTER = "<<c<<endl;
            throw std::invalid_argument("ERROR STATE4");
            break;
    }
}

void Scanner::state5(fstream& in)
{
    struct token t;
    t.cat = INST;
    t.value = store;
    v.push_back(t);
    return;
}

void Scanner::state6(fstream& in)
{
    char c;
    in.get(c);
    switch(c){
        case 'b':
            state7(in);
            break;
        default:
            cout<<"INVALID CHARACTER = "<<c<<endl;
            throw std::invalid_argument("ERROR STATE6");
            break;
    }
}

void Scanner::state7(fstream& in)
{
    token t;
    t.cat =INST;
    t.value = sub;
    v.push_back(t);
     return;
}

void Scanner::state8(fstream& in)
{
    char c;
    in.get(c);
    switch(c){
        case 'o':
            state9(in);
            break;
        case 's':
            state14(in, 'l');
            break;
        default:
            cout<<"INVALID CHARACTER = "<<c<<endl;
            throw std::invalid_argument("ERROR STATE8");
            break;
    }
}

void Scanner::state9(fstream& in)
{
    char c;
    in.get(c);
    switch(c){
        case 'a':
            state10(in);
            break;
        default:
            cout<<"INVALID CHARACTER = "<<c<<endl;
            throw std::invalid_argument("ERROR STATE9");
            break;
    }
}

void Scanner::state10(fstream& in)
{
    char c;
    in.get(c);
    switch(c){
        case 'd':
            state11(in);
            break;
        default:
            cout<<"INVALID CHARACTER = "<<c<<endl;
            throw std::invalid_argument("ERROR STATE10");
            break;
    }
}

void Scanner::state11(fstream& in)
{
    int next = in.peek();
    if(next == ' ' || next == '\t') 
    {
        struct token t;
        t.cat = INST;
        t.value = load;
        v.push_back(t);
         return;
    }
    else{
        char c;
        in.get(c);
        switch(c){
            case 'I':
                state12(in);
                break;
            default:
                cout<<"INVALID CHARACTER = "<<c<<endl;
                throw std::invalid_argument("ERROR STATE11");
                break;
        }
    }
}

void Scanner::state12(fstream& in)
{
    struct token t;
    t.cat = INST;
    t.value = loadI;
    v.push_back(t);
     return;
}


void Scanner::state13(fstream& in)
{
    char c; 
    in.get(c);
    if(isdigit(c)){
        int n = (c - '0');
        registerState1(in,n);
    } else {
        switch(c){
            case 's':
                state14(in,'r');
                break;
            default:
                cout<<"INVALID CHARACTER = "<<c<<endl;
                throw std::invalid_argument("ERROR STATE13");
                break;
        }
    }
}

void Scanner::state14(fstream& in,char back)
{
    char c;
    in.get(c);
    switch(c){
        case 'h':
            state15(in,back);
            break;
        default:
            cout<<"INVALID CHARACTER = "<<c<<endl;
            throw std::invalid_argument("ERROR STATE14");
            break;
    }
}

void Scanner::state15(fstream& in,char back)
{
    char c;
    in.get(c);
    switch(c){
        case 'i':
            state16(in,back);
            break;
        default:
            cout<<"INVALID CHARACTER = "<<c<<endl;
            throw std::invalid_argument("ERROR STATE15");
            break;
    }
}

void Scanner::state16(fstream& in,char back)
{
    char c;
    in.get(c);
    switch(c){
        case 'f':
            state17(in,back);
            break;
        default:
            cout<<"INVALID CHARACTER = "<<c<<endl;
            throw std::invalid_argument("ERROR STATE16");
            break;
    }
}

void Scanner::state17(fstream& in, char back)
{
    char c;
    in.get(c);
    switch(c){
        case 't':
            state18(in,back);
            break;
        default:
            cout<<"INVALID CHARACTER = "<<c<<endl;
            throw std::invalid_argument("ERROR STATE17");
            break;
    }
}

void Scanner::state18(fstream& in,char back)
{
    struct token t;
    t.cat = INST;
    if(back == 'r') t.value = rshift;
    if(back == 'l') t.value = lshift;
    v.push_back(t);
     return;
}

void Scanner::state18Mult(fstream& in)
{
    struct token t;
    t.cat = INST; 
    t.value = mult;
    v.push_back(t);
    return;
}

void Scanner::state19(fstream& in)
{
    char c;
    in.get(c);
    switch(c){
        case 'u':
            state20(in);
            break;
        default:
            cout<<"INVALID CHARACTER = "<<c<<endl;
            throw std::invalid_argument("ERROR STATE19");
            break;
    }
}

void Scanner::state20(fstream& in)
{
    char c;
    in.get(c);
    switch(c){
        case 'l':
            state21(in);
            break;
        default:
            cout<<"INVALID CHARACTER = "<<c<<endl;
            throw std::invalid_argument("ERROR STATE20");
            break;
    }
}

void Scanner::state21(fstream& in)
{
    char c;
    in.get(c);
    switch(c){
        case 't':
            state18Mult(in);
            break;
        default:
            cout<<"INVALID CHARACTER = "<<c<<endl;
            throw std::invalid_argument("ERROR STATE21");
            break;
    }
}

void Scanner::state22(fstream& in)
{
    char c;
    in.get(c);
    switch(c){
        case 'd':
            state23(in);
            break;
        default:
            cout<<"INVALID CHARACTER = "<<c<<endl;
            throw std::invalid_argument("ERROR STATE22");
            break;
    }
}

void Scanner::state23(fstream& in)
{
    char c;
    in.get(c);
    switch(c){
        case 'd':
            state24(in);
            break;
        default:
            cout<<"INVALID CHARACTER = "<<c<<endl;
            throw std::invalid_argument("ERROR STATE23");
            break;
    }
}

void Scanner::state24(fstream& in)
{
    struct token t;
    t.cat = INST;
    t.value = add;
    v.push_back(t);
     return;
}

void Scanner::state25(fstream& in)
{
    char c;
    in.get(c);
    switch(c){
        case 'o':
            state26(in);
            break;
        default:
            cout<<"INVALID CHARACTER = "<<c<<endl;
            throw std::invalid_argument("ERROR STATE25");
            break;
    }
}

void Scanner::state26(fstream& in)
{
    char c;
    in.get(c);
    switch(c){
        case 'p':
            state27(in);
            break;
        default:
            cout<<"INVALID CHARACTER = "<<c<<endl;
            throw std::invalid_argument("ERROR STATE26");
            break;
    }
}

void Scanner::state27(fstream& in)
{
    struct token t;
    t.cat = INST;
    t.value = nop;
    v.push_back(t);
     return;
}

void Scanner::state28(fstream& in  )
{
    char c;
    in.get(c);
    switch(c){
        case 'u':
            state29(in);
            break;
        default:   
            cout<<"INVALID CHARACTER = "<<c<<endl;
            throw std::invalid_argument("ERROR STATE28");
            break;
    }
}

void Scanner::state29(fstream& in)
{
    char c;
    in.get(c);
    switch(c){
        case 't':
            state30(in);
            break;
        default:
            cout<<"INVALID CHARACTER = "<<c<<endl;
            throw std::invalid_argument("ERROR STATE29");
            break;
    }
}

void Scanner::state30(fstream& in)
{
    char c;
    in.get(c);
    switch(c){
        case 'p':
            state31(in);
            break;
        default:
            cout<<"INVALID CHARACTER = "<<c<<endl;
            throw std::invalid_argument("ERROR STATE30");
            break;
    }
}

void Scanner::state31(fstream& in)
{
    char c;
    in.get(c);
    switch(c){
        case 'u':
            state32(in);
            break;
        default:
            cout<<"INVALID CHARACTER = "<<c<<endl;
            throw std::invalid_argument("ERROR STATE31");
            break;
    }
}

void Scanner::state32(fstream& in)
{
    char c;
    in.get(c);
    switch(c){
        case 't':
            state33(in);
            break;
        default:
            cout<<"INVALID CHARACTER = "<<c<<endl;
            throw std::invalid_argument("ERROR STATE32");
            break;
    }
}

void Scanner::state33(fstream& in)
{
    struct token t;
    t.cat = INST;
    t.value = output;
    v.push_back(t);
     return;
}