#include "../include/lexer.hpp"
#include "../include/parser.hpp"
#include "../include/codegen.hpp"

int main(int argc, char** argv) {
    if (argc < 2) {
        cout << "Usage: uvm [input_file] [output_file]\n" << endl;
        return -1;
    }
    const char* in = argv[1];
    const char* out = argv[2];

    Lexer lexer(in);
    lexer.tokenize();

    //for (auto& t : lexer.get_tokens()) {
    //    t.print();
    //}

    Parser parser(lexer.get_tokens());
    parser.parse();


    Compiler compiler(parser.get_labels(), out);
    compiler.compile();

    printf("n_tokens: %ld\n", lexer.get_tokens().size());

    return 0;
}
