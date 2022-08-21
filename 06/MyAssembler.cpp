#include <iostream>
#include <string> 
#include <fstream>
#include <vector>
#include <sstream>
#include <map>

using namespace std;

bool endsWith(string s1, string s2) {
    int s1_length = s1.length();
    if(s1_length < 5) { //x.asm must be at least 5 characters
        cerr << "x.asm must be at least 5 characters\n";
        exit(2);
    }
    string s1_end = s1.substr(s1_length - 4, 4);

    return (s1_end == ".asm");
}

class Parser{
private:
    map<string, int> theMap;
    enum COMMAND_TYPE{
        A_COMMAND = 0,
        C_COMMAND,
        L_COMMAND
    };
    ifstream theFile;
    ofstream newFile;
    string buffer;
    vector<string> stripped_commands;
    vector<string> translated_commands;
    string current_command;
    COMMAND_TYPE commandType();
public:
    void parse();
    string translateAcommand();
    string translateCcommand();
    void stripInput(string fileName, string newFileName);
    string toBinary(int n);
    void printTranslated();
    void translateSymbolless();
    void convertToSymbolless();
        void initialize();
        void firstPass();
        void secondPass();
    void printStripped() {
        for(auto s : stripped_commands) {
            cout << s << endl;
        }
    }

};
void Parser::printTranslated(){

    for(int i = 0; i < translated_commands.size(); i++){
        cout << translated_commands.at(i) << endl;
    }    
}

void Parser::initialize(){
    for(int i = 0; i < 16; i++){
        string Rnum = to_string(i);
        theMap.insert(pair<string, int>("R" + Rnum, i));
    }
    theMap.insert(pair<string, int>("SP", 0));
    theMap.insert(pair<string, int>("LCL", 1));
    theMap.insert(pair<string, int>("ARG", 2));
    theMap.insert(pair<string, int>("THIS", 3));
    theMap.insert(pair<string, int>("THAT", 4));
    theMap.insert(pair<string, int>("SCREEN", 16384));
    theMap.insert(pair<string, int>("KBD", 24576));


}

void Parser::firstPass(){
    //stripped_commands
    int stripComSize = stripped_commands.size();
    int romCounter = 0;
    for(int i = 0; i < stripComSize; i++){
        current_command = stripped_commands.at(i);
        COMMAND_TYPE type = commandType();
        switch(type){
            case A_COMMAND:
            case C_COMMAND:
                romCounter++;
                break;
            case L_COMMAND:
                int LComSize = current_command.size();
                string LCom = current_command.substr(1, LComSize - 2);
                theMap.insert(pair<string, int>(LCom, romCounter));
        }
    }
}
void Parser::secondPass(){
    int stripComSize = stripped_commands.size();
    int ramCounter = 16;
    for(int i = 0; i < stripComSize; i++){
        current_command = stripped_commands.at(i);
        COMMAND_TYPE type = commandType();
        switch (type) {
            case A_COMMAND: {
                string LCom = current_command.substr(1, current_command.length() - 1);
                if(theMap.find(LCom) != theMap.end()){
                    stripped_commands[i] = "@" + to_string(theMap[LCom]);
                }
                else{
                    if(!(isdigit(LCom.at(0)))){
                        theMap.insert(pair<string, int>(LCom, ramCounter++));
                        stripped_commands[i] = "@" + to_string(theMap[LCom]);
                    }
                }
            }
                break;
            case C_COMMAND:
                break;
            case L_COMMAND:
                break;
        }
    }
}

void Parser::convertToSymbolless(){
    initialize();
    firstPass();
    secondPass();
}

void Parser::stripInput(string fileName, string newFileName){
    theFile.open(fileName);
    if(theFile.bad()) {
        exit(1);
    }
    newFile.open(newFileName);
    while(getline(theFile, buffer)){
        int bufferLen = buffer.length();
        if(bufferLen < 2){
            continue;
        }
        if(buffer.at(0) == '/' && buffer.at(1) == '/'){
            continue;
        }
        remove(buffer.begin(), buffer.end(), ' ');
        // remove(buffer.begin(), buffer.end(), (char)13);
        // remove(buffer.begin(), buffer.end(), '\t');
        bufferLen = buffer.length();
        string s = "";
        for(int i = 0; i < bufferLen - 1; i++){
            
            if((buffer.at(i) == '/' && buffer.at(i+1) == '/')){
                break;
            }
            char c = buffer.at(i);
            s.push_back(c);

        }
        stripped_commands.push_back(s);

    }
    
    //clean any remaining whitespace
    for(int i = 0; i < stripped_commands.size(); i++) {
        stringstream ss(stripped_commands[i]);
        string s;
        ss >> s;
        stripped_commands[i] = s;
    }
    theFile.close();
    newFile.close();
}//this simply creates a new file which has just the intsructions

string Parser::toBinary(int n){
    string bits = "";

    int current = n;
    while(current){
        if(current % 2){
            bits += "1";
        }
        else{
            bits += "0";
        }
        current /= 2;
    }
    int numZeros = 15 - bits.length();
    for(int i  = 0; i < numZeros; i++){
        bits += "0";
    }

    if(bits.length() != 15){
        cerr << "added wrong number of zeroes";
        exit(3);
    }
    else{
        reverse(bits.begin(), bits.end());

        return bits;
    }

}


string Parser::translateAcommand(){
    string LMB = "0";
    string copy = current_command.substr(1);
    int input = stoi(copy);
    
    string binaryNum = toBinary(input); 
    return LMB + binaryNum; 
}

string Parser::translateCcommand(){
    string L3MB = "111";
    string jmpBits ="000" ;
    string compBits = "";
    string destBits ="000" ;
    string leftSide = "";
    string rightSide = "";
    
    int index = current_command.find("=");
    if(index >= 0){
        leftSide = current_command.substr(0, (index));
        rightSide = current_command.substr((index + 1)/*,current_command.length() - (index + 1) */);
        if(leftSide == "M"){
            destBits = "001";
        }
        else if(leftSide == "D"){
            destBits = "010";
        }
        else if(leftSide == "MD"){
            destBits = "011";
        }
        else if(leftSide == "A"){
            destBits = "100";
        }
        else if(leftSide == "AM"){
            destBits = "101";
        }
        else if(leftSide == "AD"){
            destBits = "110";
        }
        else if(leftSide == "AMD"){
            destBits = "111";
        }
        else {
            cout << "didn't take one of the branches\n";
            exit(3);
        }
        //-------------------------------------------
        if(rightSide == "0"){
            compBits = "0101010";
        }
        else if(rightSide == "1"){
            compBits = "0111111";
        }
        else if(rightSide == "-1"){
            compBits = "0111010";
        }
        else if(rightSide == "D"){
            compBits = "0001100";
        }
        else if(rightSide == "A"){
            compBits = "0110000";
        }
        else if(rightSide == "!D"){
            compBits = "0001101";
        }
        else if(rightSide == "!A"){
            compBits = "0110001";
        }
        else if(rightSide == "-D"){
            compBits = "0001111";
        }
        else if(rightSide == "-A"){
            compBits = "0110011";
        }
        else if(rightSide == "D+1"){
            compBits = "0011111";
        }
        else if(rightSide == "A+1"){
            compBits = "0110111";
        }
        else if(rightSide == "D-1"){
            compBits = "0001110";
        }
        else if(rightSide == "A-1"){
            compBits = "0110010";
        }
        else if(rightSide == "D+A"){
            compBits = "0000010";
        }
        else if(rightSide == "D-A"){
            compBits = "0010011";
        }
        else if(rightSide == "A-D"){
            compBits = "0000111";
        }
        else if(rightSide == "D&A"){
            compBits = "0000000";
        }
        else if(rightSide == "D|A"){
            compBits = "0010101";
        }
        else if(rightSide == "M"){
            compBits = "1110000";
        }
        else if(rightSide == "!M"){
            compBits = "1110001";
        }
        else if(rightSide == "-M"){
            compBits = "1110011";
        }
        else if(rightSide == "M+1"){
            compBits = "1110111";
        }
        else if(rightSide == "M-1"){
            compBits = "1110010";
        }
        else if(rightSide == "D+M"){
            compBits = "1000010";
        }
        else if(rightSide == "D-M"){
            compBits = "1010011";
        }
        else if(rightSide == "M-D"){
            compBits = "1000111";
        }
        else if(rightSide == "D&M"){
            compBits = "1000000";
        }
        else if(rightSide == "D|M"){
            compBits = "1010101";
        }
        else {
            cout << "didn't take one of the branches2\n";
            exit(3);
        }    
        return L3MB + compBits + destBits + jmpBits;
    }
    
    index = current_command.find(";");
    if(index >= 0) {
        leftSide = current_command.substr(0, (index));
        rightSide = current_command.substr((index + 1)/*,current_command.length() - (index + 1) */);
        if(leftSide == "0"){
            compBits = "0101010";
        }
        else if(leftSide == "1"){
            compBits = "0111111";
        }
        else if(leftSide == "-1"){
            compBits = "0111010";
        }
        else if(leftSide == "D"){
            compBits = "0001100";
        }
        else if(leftSide == "A"){
            compBits = "0110000";
        }
        else if(leftSide == "!D"){
            compBits = "0001101";
        }
        else if(leftSide == "!A"){
            compBits = "0110001";
        }
        else if(leftSide == "-D"){
            compBits = "0001111";
        }
        else if(leftSide == "-A"){
            compBits = "0110011";
        }
        else if(leftSide == "D+1"){
            compBits = "0011111";
        }
        else if(leftSide == "A+1"){
            compBits = "0110111";
        }
        else if(leftSide == "D-1"){
            compBits = "0001110";
        }
        else if(leftSide == "A-1"){
            compBits = "0110010";
        }
        else if(leftSide == "D+A"){
            compBits = "0000010";
        }
        else if(leftSide == "D-A"){
            compBits = "0010011";
        }
        else if(leftSide == "A-D"){
            compBits = "0000111";
        }
        else if(leftSide == "D&A"){
            compBits = "0000000";
        }
        else if(leftSide == "D|A"){
            compBits = "0010101";
        }
        else if(leftSide == "M"){
            compBits = "1110000";
        }
        else if(leftSide == "!M"){
            compBits = "1110001";
        }
        else if(leftSide == "-M"){
            compBits = "1110011";
        }
        else if(leftSide == "M+1"){
            compBits = "1110111";
        }
        else if(leftSide == "M-1"){
            compBits = "1110010";
        }
        else if(leftSide == "D+M"){
            compBits = "1000010";
        }
        else if(leftSide == "D-M"){
            compBits = "1010011";
        }
        else if(leftSide == "M-D"){
            compBits = "1000111";
        }
        else if(leftSide == "D&M"){
            compBits = "1000000";
        }
        else if(leftSide == "D|M"){
            compBits = "1010101";
        }
        else{
            cerr << "did not take any branch ;;;;" << endl;
        }
        //-------------------------------
        if(rightSide == "JGT"){
            jmpBits = "001";
        }
        else if(rightSide == "JEQ"){
            jmpBits = "010";
        }
        else if(rightSide == "JGE"){
            jmpBits = "011";
        }
        else if(rightSide == "JLT"){
            jmpBits = "100";
        }
        else if(rightSide == "JNE"){
            jmpBits = "101";
        }
        else if(rightSide == "JLE"){
            jmpBits = "110";
        }
        else if(rightSide == "JMP"){
            jmpBits = "111";
        }
        return L3MB + compBits + destBits + jmpBits;
    }

    cout << "not a valid C instruction\n";
    exit(1);

    
    
    
}

Parser::COMMAND_TYPE Parser::commandType() {
    char c = current_command.at(0);
    if(c == '@')        return A_COMMAND;
    else if(c == '(')   return L_COMMAND;
    else                return C_COMMAND;
}   

void Parser::translateSymbolless(){
    for(int i = 0; i < stripped_commands.size(); i++) {
        current_command = stripped_commands[i];
        
        COMMAND_TYPE type = commandType();
        switch(type) {
        case A_COMMAND:
            translated_commands.push_back(translateAcommand());
            break;
        case L_COMMAND:
            
            break;
        case C_COMMAND:
            translated_commands.push_back(translateCcommand());
            break;
        

        }
    }
}

void Parser::parse() {
    convertToSymbolless();
    translateSymbolless();
}

int main(int argc, char* argv[]){
    if(argc != 2) {
        cerr << "Must provide a single argument to the assembler\n";
        exit(1);
    }

    string asm_prog_name = argv[1];

    if(!endsWith(asm_prog_name, ".asm")) {
        cerr << "Must provide a .asm file\n";
    }
    Parser parser;
    parser.stripInput(asm_prog_name, "theNewFile.asm");
    // parser.printStripped();
    parser.parse();
    parser.printTranslated();


    
    //cout << "Hi guys" << endl;
    
    return 0;
}
