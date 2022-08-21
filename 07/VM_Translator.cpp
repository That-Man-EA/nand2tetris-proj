#include <iostream>
#include <vector>
#include <string>
#include <dirent.h>
#include <fstream>
#include <sstream>

using namespace std;

vector<string> vm_files;
vector<vector<string> > current_stripped_input;
vector<string> hack_output;
ifstream current_ifs;
int label_counter = 0;

/**Assume that inline comments are allowed, but they will be 
 * separated out by a space. For example, 
 * push constant 1 //someComment
 * is allowed but 
 * push constant 1//someComment
 * is now allowed */
void strip_input(string file) {
    current_stripped_input.clear();
    current_ifs.open(file);
    string line;
    while(getline(current_ifs, line)) {
        vector<string> words;
        //empty line
        if(line.length() == 1) {
            continue;
        }
        stringstream ss(line);
        string cleaned_line;
        string current_word;
        ss >> current_word;

        bool broke = false;
        while(!ss.eof()) {
            if(current_word.find("//") == 0) {
                broke = true;
                break;
            }
            words.push_back(current_word);
            ss >> current_word;
        }
        if(words.size()) {
            current_stripped_input.push_back(words);
        }
        
    }
    current_ifs.close();
}

void discover_vm_files(string arg) {
    DIR* dir;
    struct dirent* dir_entry;

    errno = 0;
    if((dir = opendir(arg.c_str())) == NULL) {
        switch(errno) {
        case EACCES: 
            printf("Permission denied\n"); 
            break;
        case ENOENT: 
            printf("Directory does not exist\n"); 
            break;
        case ENOTDIR:
            printf("%s is not a directory\n", arg.c_str()); 
            break;
        }
        exit(EXIT_FAILURE);
    }

    bool path_ends_in_slash = (*(arg.end() - 1) == '/');
    while((dir_entry = readdir(dir)) != nullptr) {
        string filename = dir_entry->d_name;
        string::iterator it = filename.end();
        bool is_vm_file = (
            (*(it - 3) == '.') &&
            (*(it - 2) == 'v') &&
            (*(it - 1) == 'm')
        );
        if(is_vm_file) {
            if(path_ends_in_slash) {
                vm_files.push_back(arg + filename);
            }
            else {
                vm_files.push_back(arg + "/" + filename);
            }
        }
    }    
}

bool valid_commandline_invocation(int argc, char** argv) {
    if(argc != 2) {
        cout << "Must provide a single argument representing either a directory or source file\n";
        return false;
    }

    //assume anything that doesn't end in .vm is a directory
    //otherwise it's a vm file
    bool is_dir = true;
    string arg = argv[1];

    //x.vm must be at least 4 characters, and has to end in .vm
    if(arg.size() >= 4) {
        string::iterator it = arg.end();
        is_dir = !(
            (*(it - 3) == '.') &&
            (*(it - 2) == 'v') &&
            (*(it - 1) == 'm')
        ); 
    }

    if(is_dir) {
        discover_vm_files(arg);
    }
    else {
        vm_files.push_back(arg);
    }
    
    
    return true;
}

void do_arithmetic(string& the_command){
    if(the_command == "add"){
        //start the add command
        hack_output.push_back("@SP"); //should start with 256, but with the simpleAdd.vm
        hack_output.push_back("D=M"); //this should be 258 right now^^^
        hack_output.push_back("D=D-1"); //D=257
        hack_output.push_back("A=D"); //A = 257
        hack_output.push_back("D=M"); //D = 257
        hack_output.push_back("@R15"); //temp variable
        hack_output.push_back("M=D"); //M = 257 we got this topmost number and put it into a temp variable so far

        hack_output.push_back("@SP");
        hack_output.push_back("D=M"); //D = 258
        hack_output.push_back("D=D-1"); //D = 257
        hack_output.push_back("D=D-1"); //D = 256, aka where the first number resides
        hack_output.push_back("A=D"); // A = 256
        hack_output.push_back("D=M"); // D = 256

        hack_output.push_back("@R15"); //call the last temp variable
        hack_output.push_back("D=D+M"); // add the temp value plus the current D value, which should be 256 + 257
        //D has D = RAM[256] + RAM[257]
        hack_output.push_back("@SP"); //M=258
        hack_output.push_back("A=M"); //A=258
        hack_output.push_back("A=A-1"); //A=257
        hack_output.push_back("A=A-1"); //A=256
        hack_output.push_back("M=D"); //M[A] = D | D = RAM[256] + RAM[257]
        hack_output.push_back("@SP"); //call SP so M = 258
        hack_output.push_back("M=M-1"); //move it back once because this is now the next available space the write in, not matter what is has, pretty sure it still has the secone thing we pushed but it does not matter since it is just garbage value.
        //end the add command
    }
    else if(the_command == "sub"){//this one should deadass be the exact same thing just that now we do D = RAM[257] - RAM[256], and then place everything in RAM[256]
        hack_output.push_back("@SP"); 
        hack_output.push_back("D=M"); 
        hack_output.push_back("D=D-1");
        hack_output.push_back("A=D");
        hack_output.push_back("D=M");

        hack_output.push_back("@R15"); 
        hack_output.push_back("M=D");

        hack_output.push_back("@SP");
        hack_output.push_back("D=M");
        hack_output.push_back("D=D-1");
        hack_output.push_back("D=D-1");
        hack_output.push_back("A=D");
        hack_output.push_back("D=M");

        hack_output.push_back("@R15");
        hack_output.push_back("D=D-M");

        hack_output.push_back("@SP");
        hack_output.push_back("A=M");
        hack_output.push_back("A=A-1");
        hack_output.push_back("A=A-1");
        hack_output.push_back("M=D");

        hack_output.push_back("@SP");
        hack_output.push_back("M=M-1");
    }
    else if(the_command == "neg"){
        hack_output.push_back("@SP");
        hack_output.push_back("A=M");
        hack_output.push_back("A=A-1");
        hack_output.push_back("M=-M");
    }
    else if(the_command == "eq"){
        hack_output.push_back("@SP");
        hack_output.push_back("A=M-1");
        hack_output.push_back("D=M");
        hack_output.push_back("A=A-1");
        hack_output.push_back("A=M");
        hack_output.push_back("D=D-A");

        string label = "jumpwhenDis0" + to_string(label_counter++);
        string label2 = "Dwasnt0" + to_string(label_counter++);
        hack_output.push_back("@" + label);
        hack_output.push_back("D;JEQ");
        hack_output.push_back("D=0");
        hack_output.push_back("@" + label2);
        hack_output.push_back("0;JMP");
        
        hack_output.push_back("(" + label + ")");
        hack_output.push_back("D=-1");
        hack_output.push_back("(" + label2 + ")");
        hack_output.push_back("@SP");
        hack_output.push_back("A=M-1");
        hack_output.push_back("A=A-1");
        hack_output.push_back("M=D");

        hack_output.push_back("@SP");
        hack_output.push_back("M=M-1");

    }
    else if(the_command == "lt"){
        hack_output.push_back("@SP");
        hack_output.push_back("A=M-1");
        hack_output.push_back("D=M");
        hack_output.push_back("A=A-1");
        hack_output.push_back("A=M");
        hack_output.push_back("D=A-D");

        string label = "jumpwhenDisLess0" + to_string(label_counter++);
        hack_output.push_back("@" + label);
        hack_output.push_back("D;JLT");
        hack_output.push_back("D=0");
        hack_output.push_back("@wasntLessThan0" + to_string(label_counter));
        hack_output.push_back("0;JMP");


        hack_output.push_back("(" + label + ")");
        hack_output.push_back("D=-1");
        hack_output.push_back("(wasntLessThan0" + to_string(label_counter) + ")");

        hack_output.push_back("@SP");
        hack_output.push_back("A=M-1");
        hack_output.push_back("A=A-1");
        hack_output.push_back("M=D");

        hack_output.push_back("@SP");
        hack_output.push_back("M=M-1");
    }
    else if(the_command == "gt"){
        hack_output.push_back("@SP");
        hack_output.push_back("A=M-1");
        hack_output.push_back("D=M");
        hack_output.push_back("A=A-1");
        hack_output.push_back("A=M");
        hack_output.push_back("D=A-D");

        string label = "jumpwhenDisMore0" + to_string(label_counter++);
        hack_output.push_back("@" + label);
        hack_output.push_back("D;JGT");
        hack_output.push_back("D=0");
        hack_output.push_back("@wasntMoreThan0" + to_string(label_counter));
        hack_output.push_back("0;JMP");


        hack_output.push_back("(" + label + ")");
        hack_output.push_back("D=-1");
        hack_output.push_back("(wasntMoreThan0" + to_string(label_counter) + ")");

        hack_output.push_back("@SP");
        hack_output.push_back("A=M-1");
        hack_output.push_back("A=A-1");
        hack_output.push_back("M=D");

        hack_output.push_back("@SP");
        hack_output.push_back("M=M-1");
    }
    else if(the_command == "and"){//complete this one---------------------
        hack_output.push_back("@SP");
        hack_output.push_back("A=M-1");
        hack_output.push_back("D=M");
        hack_output.push_back("A=A-1");
        hack_output.push_back("M=M&D");
        hack_output.push_back("@SP");
        hack_output.push_back("M=M-1");


    }
    else if(the_command == "or"){//complete this one---------------------
        hack_output.push_back("@SP");
        hack_output.push_back("A=M-1");
        hack_output.push_back("D=M");
        hack_output.push_back("A=A-1");
        hack_output.push_back("M=M|D");
        hack_output.push_back("@SP");
        hack_output.push_back("M=M-1");
        

    }
    else if(the_command == "not"){//complete this one---------------------
        hack_output.push_back("@SP");
        hack_output.push_back("A=M-1");
        hack_output.push_back("M=!M");
    }
    //and
    //or
    //not
    
}

void do_push_or_pop(const string& memCom, const string& segment, const string& index){
    
    if(memCom == "push" && segment == "constant"){
        hack_output.push_back("@" + index); //A now has the index
        hack_output.push_back("D=A"); //D now has the index
        hack_output.push_back("@SP"); //A is zero because thats where SP starts, value of SP always starts at 256 so M has 256
        hack_output.push_back("A=M"); //A is now 256
        hack_output.push_back("M=D"); //M gets the index now
        hack_output.push_back("@SP"); //call SP to ++1 on it
        hack_output.push_back("M=M+1"); //the ++1 done being done.
        
    }
    else if(memCom == "push" && segment == "local"){
        hack_output.push_back("@LCL");
        hack_output.push_back("D=M");
        hack_output.push_back("@" + index);
        hack_output.push_back("D=A+D");
        hack_output.push_back("A=D");
        hack_output.push_back("D=M");
        hack_output.push_back("@SP");
        hack_output.push_back("A=M");    
        hack_output.push_back("M=D");
        hack_output.push_back("@SP");
        hack_output.push_back("M=M+1");
        //complete sometime othertime, this is if memory access command is a 'pop'

    }
    else if(memCom == "pop" && segment == "local"){
        hack_output.push_back("@LCL");
        hack_output.push_back("D=M");
        hack_output.push_back("@" + index);
        hack_output.push_back("D=A+D");
        hack_output.push_back("@R13");
        hack_output.push_back("M=D");
        hack_output.push_back("@SP");
        hack_output.push_back("A=M-1");
        hack_output.push_back("D=M");
        hack_output.push_back("@R13");
        hack_output.push_back("A=M");
        hack_output.push_back("M=D");
        hack_output.push_back("@SP");
        hack_output.push_back("M=M-1");
    }
    else if(memCom == "pop" && segment == "argument"){
        hack_output.push_back("@ARG");
        hack_output.push_back("D=M");
        hack_output.push_back("@" + index);
        hack_output.push_back("D=A+D");
        hack_output.push_back("@R15");
        hack_output.push_back("M=D");
        hack_output.push_back("@SP");
        hack_output.push_back("A=M-1");
        hack_output.push_back("D=M");
        hack_output.push_back("@R15");
        hack_output.push_back("A=M");
        hack_output.push_back("M=D");
        hack_output.push_back("@SP");
        hack_output.push_back("M=M-1");
    }
    else if(memCom == "pop" && segment == "this"){
        hack_output.push_back("@THIS");
        hack_output.push_back("D=M");
        hack_output.push_back("@" + index);
        hack_output.push_back("D=A+D");
        hack_output.push_back("@R15");
        hack_output.push_back("M=D");
        hack_output.push_back("@SP");
        hack_output.push_back("A=M-1");
        hack_output.push_back("D=M");
        hack_output.push_back("@R15");
        hack_output.push_back("A=M");
        hack_output.push_back("M=D");
        hack_output.push_back("@SP");
        hack_output.push_back("M=M-1");
    }
    else if(memCom == "pop" && segment == "that"){
        hack_output.push_back("@THAT");
        hack_output.push_back("D=M");
        hack_output.push_back("@" + index);
        hack_output.push_back("D=A+D");
        hack_output.push_back("@R15");
        hack_output.push_back("M=D");
        hack_output.push_back("@SP");
        hack_output.push_back("A=M-1");
        hack_output.push_back("D=M");
        hack_output.push_back("@R15");
        hack_output.push_back("A=M");
        hack_output.push_back("M=D");
        hack_output.push_back("@SP");
        hack_output.push_back("M=M-1");
    }
    else if(memCom == "pop" && segment == "temp"){

        hack_output.push_back("@R5");
        hack_output.push_back("D=A");
        hack_output.push_back("@" + index);
        hack_output.push_back("D=A+D");
        hack_output.push_back("@R13");
        hack_output.push_back("M=D");
        hack_output.push_back("@SP");
        hack_output.push_back("A=M-1");
        hack_output.push_back("D=M");
        hack_output.push_back("@R13");
        hack_output.push_back("A=M");
        hack_output.push_back("M=D");
        hack_output.push_back("@SP");
        hack_output.push_back("M=M-1");
    }
    else if(memCom == "push" && segment == "that"){
        hack_output.push_back("@THAT");
        hack_output.push_back("D=M");
        hack_output.push_back("@" + index);
        hack_output.push_back("D=A+D");
        hack_output.push_back("A=D");
        hack_output.push_back("D=M");
        hack_output.push_back("@SP");
        hack_output.push_back("A=M");
        hack_output.push_back("M=D");
        hack_output.push_back("@SP");
        hack_output.push_back("M=M+1");
    }
    else if(memCom == "push" && segment == "this"){
        hack_output.push_back("@THIS");
        hack_output.push_back("D=M");
        hack_output.push_back("@" + index);
        hack_output.push_back("D=A+D");
        hack_output.push_back("A=D");
        hack_output.push_back("D=M");
        hack_output.push_back("@SP");
        hack_output.push_back("A=M");
        hack_output.push_back("M=D");
        hack_output.push_back("@SP");
        hack_output.push_back("M=M+1");
    }
    else if(memCom == "push" && segment == "temp"){
        hack_output.push_back("@R5");
        hack_output.push_back("D=A");
        hack_output.push_back("@" + index);
        hack_output.push_back("D=A+D");
        hack_output.push_back("A=D");
        hack_output.push_back("D=M");
        hack_output.push_back("@SP");
        hack_output.push_back("A=M");
        hack_output.push_back("M=D");
        hack_output.push_back("@SP");
        hack_output.push_back("M=M+1");
    }
    else if(memCom == "push" && segment == "argument"){
        hack_output.push_back("@ARG");
        hack_output.push_back("D=M");
        hack_output.push_back("@" + index);
        hack_output.push_back("D=A+D");
        hack_output.push_back("A=D");
        hack_output.push_back("D=M");
        hack_output.push_back("@SP");
        hack_output.push_back("A=M");    
        hack_output.push_back("M=D");
        hack_output.push_back("@SP");
        hack_output.push_back("M=M+1");
    }
    else if(memCom == "push" && segment == "pointer"){
        hack_output.push_back("@R3");
        hack_output.push_back("D=A");
        hack_output.push_back("@" + index);
        hack_output.push_back("D=A+D");
        hack_output.push_back("A=D");
        hack_output.push_back("D=M");
        hack_output.push_back("@SP");
        hack_output.push_back("A=M");
        hack_output.push_back("M=D");
        hack_output.push_back("@SP");
        hack_output.push_back("M=M+1");
    }
    else if(memCom == "pop" && segment == "pointer"){
        hack_output.push_back("@R3");
        hack_output.push_back("D=A");
        hack_output.push_back("@" + index);
        hack_output.push_back("D=A+D");
        hack_output.push_back("@R15");
        hack_output.push_back("M=D");
        hack_output.push_back("@SP");
        hack_output.push_back("A=M-1");
        hack_output.push_back("D=M");
        hack_output.push_back("@R15");
        hack_output.push_back("A=M");
        hack_output.push_back("M=D");
        hack_output.push_back("@SP");
        hack_output.push_back("M=M-1");
    }
    else if(memCom == "push" && segment == "static"){
        hack_output.push_back("@16");
        hack_output.push_back("D=A");
        hack_output.push_back("@" + index);
        hack_output.push_back("D=A+D");
        hack_output.push_back("A=D");
        hack_output.push_back("D=M");
        hack_output.push_back("@SP");
        hack_output.push_back("A=M");
        hack_output.push_back("M=D");
        hack_output.push_back("@SP");
        hack_output.push_back("M=M+1");
    }
    else if(memCom == "pop" && segment == "static"){
        hack_output.push_back("@16");
        hack_output.push_back("D=A");
        hack_output.push_back("@" + index);
        hack_output.push_back("D=A+D");
        hack_output.push_back("@R15");
        hack_output.push_back("M=D");
        hack_output.push_back("@SP");
        hack_output.push_back("A=M-1");
        hack_output.push_back("D=M");
        hack_output.push_back("@R15");
        hack_output.push_back("A=M");
        hack_output.push_back("M=D");
        hack_output.push_back("@SP");
        hack_output.push_back("M=M-1");
    }
    else{
        ;
    }
}

void translate_parsed_code(){
    //taosf = total_amount_of_stripped_files
    int taosf = current_stripped_input.size();
    for(int i = 0; i < taosf; i++){
        string the_command = current_stripped_input[i][0];
        if((the_command == "add") || (the_command == "sub") || (the_command == "neg") || (the_command == "eq") || (the_command == "lt") || (the_command == "gt") || (the_command == "and") || (the_command == "not") || (the_command == "or")){
            do_arithmetic(the_command);
        }
        else if((the_command == "push") || (the_command == "pop")){
            do_push_or_pop(current_stripped_input[i][0], current_stripped_input[i][1], current_stripped_input[i][2]);
        }
    }
}

int main(int argc, char** argv) {
    if(!valid_commandline_invocation(argc, argv)) {
        exit(1);
    }
    
    for(int k = 0; k < vm_files.size(); k++) {
        strip_input(vm_files[k]);
        for(int i = 0; i < vm_files.size(); i++) {
            translate_parsed_code();
        }
    }
    
    for(int i = 0; i < hack_output.size(); i++){
        cout << hack_output[i] << endl;
    }
    

}