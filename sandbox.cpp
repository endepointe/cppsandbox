#include <iostream>
#include <string>


int stringSize(std::string s)
{
    int size = 0;
    for (int i = 0; i < s.length(); i++)
    {
        size++;
    }
    return size;
}

int main(void)
{
    std::string name = "hello";
    int result = stringSize(name);
    std::cout << "Size: " << result << std::endl;

    return 0;
}
