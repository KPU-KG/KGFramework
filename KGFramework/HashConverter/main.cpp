#include "hash.h"
#include <iostream>

int main()
{
    while ( std::cin )
    {
        std::string str;
        std::cin >> str;
        std::cout << str << " : " << KG::Utill::HashString(str).value << std::endl;
    }
}