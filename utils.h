#include <string>
#include <sstream>
#include <iomanip>
#include <vector>
#include "../json/json.hpp"
#include <iostream>

struct InlineKeyboardButton{
    std::string text;
    std::string callback_data;
};

struct Row{
    std::vector<InlineKeyboardButton> buttons;
};

struct Keyboard{
    
    std::string to_string(std::vector<Row> rows){
        nlohmann::json keyboard;
        keyboard["inline_keyboard"] = nlohmann::json::array();

        short row_counter = 0;

        for (Row row : rows){
            keyboard["inline_keyboard"][row_counter] = nlohmann::json::array();

            for (InlineKeyboardButton button : row.buttons){
                

                nlohmann::json button1;
                button1["text"] = button.text;
                button1["callback_data"] = button.callback_data;
                keyboard["inline_keyboard"][row_counter].push_back(button1);
            }
            ++row_counter;
        }
        return to_serialize(keyboard.dump());
    }

    std::string to_serialize(const std::string &value){
        std::ostringstream escaped;
        escaped.fill('0');
        escaped << std::hex;

        for (char c : value) {
            if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
                escaped << c;
            } else {
                escaped << '%' << std::setw(2) << int((unsigned char) c);
            }
        }
        return escaped.str();
       
    }
};