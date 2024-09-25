

void update(){
    while (true){
        // get updates
        nlohmann::json updates =  bot.send_request("/getupdates?offset=" + std::to_string(last_update_id + 1));
        for (int index = 0; index < updates["result"].size(); index++){
            std::cout << updates["result"][index] << "\n";
            bot.handle_update(updates["result"][index]);
            last_update_id = updates["result"][index]["update_id"];
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(70));
    }
}