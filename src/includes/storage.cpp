#include "storage.hpp"
#include "user.hpp"
#include "utils/utils.hpp"
#include "follow.hpp"
#include "post.hpp"
#include "comment.hpp"
#include "notification.hpp"
#include <sstream>

// Template implementations
template <typename T>
std::string Storage::read_file_content(const fs::path &file_path) {
    std::ifstream file(file_path);
    if (!file.is_open()) {
        std::cerr << clustro::Color::RED << "[STORAGE] Failed to open: " << file_path << clustro::Color::RESET << std::endl;
        return "";
    }

    std::cout << clustro::Color::GREEN << "[STORAGE] Reading file: " << file_path << clustro::Color::RESET << std::endl;

    // Use seekg to get file size and reserve space
    file.seekg(0, std::ios::end);
    std::string content;
    content.reserve(file.tellg());
    file.seekg(0, std::ios::beg);

    content.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
    return content;
}

template <typename T>
clustro::LinkedList<T> Storage::load_items(const std::string &subdir) {
    clustro::LinkedList<T> items;
    std::string dir_path = path + "/" + subdir;

    if (!fs::exists(dir_path)) {
        std::cerr << clustro::Color::RED << "[STORAGE] Directory does not exist: " << dir_path << clustro::Color::RESET << std::endl;
        return items;
    }

    for (const auto &entry : fs::directory_iterator(dir_path)) {
        if (entry.is_regular_file() && entry.path().extension() == ".txt") {
            std::string content = read_file_content<T>(entry.path());
            if (!content.empty()) {
                try {
                    T item(content);
                    items.insert(item);
                }
                catch (const std::exception &e) {
                    std::cerr << clustro::Color::RED << "[STORAGE] Error parsing " << entry.path() << ": " << e.what() << clustro::Color::RESET << std::endl;
                }
            }
        }
    }
    return items;
}

template <typename T>
bool Storage::save_item(T &item, const std::string &subdir, const std::string &id) {
    try {
        std::string item_str = item.stringify();
        std::string outputFilePath = this->path + "/" + subdir + "/" + id + ".txt";

        std::ofstream outputFile(outputFilePath);
        if (!outputFile.is_open()) {
            std::cerr << clustro::Color::RED << "[STORAGE] Failed to open file: " << outputFilePath << clustro::Color::RESET << std::endl;
            return false;
        }

        outputFile << item_str;
        std::cout << clustro::Color::GREEN << "[STORAGE] Data saved in: " << outputFilePath << clustro::Color::RESET << std::endl;
        return true;
    }
    catch (const std::exception &e) {
        std::cerr << clustro::Color::RED << "[STORAGE] Error saving item: " << e.what() << clustro::Color::RESET << std::endl;
        return false;
    }
}

template <typename T>
bool Storage::delete_item(const std::string &subdir, const std::string &id) {
    try {
        std::string filePath = this->path + "/" + subdir + "/" + id + ".txt";
        if (fs::exists(filePath)) {
            fs::remove(filePath);
            std::cout << clustro::Color::GREEN << "[STORAGE] Deleted file: " << filePath << clustro::Color::RESET << std::endl;
            return true;
        }
        return false;
    }
    catch (const fs::filesystem_error &e) {
        std::cerr << clustro::Color::RED << "[STORAGE] Error deleting file: " << e.what() << clustro::Color::RESET << std::endl;
        return false;
    }
}

// Non-template implementations
Storage::Storage(const std::string &path) : success(false), path(path) {
    try {
        std::string sub_folders[] = {"Users", "Posts", "Comments", "Notifications", "Follow"};
        for (const std::string &sub_folder : sub_folders) {
            fs::create_directories(path + "/" + sub_folder);
            std::cout << clustro::Color::GREEN << "[STORAGE] Folder created " + path + "/" + sub_folder + "." << clustro::Color::RESET << std::endl;
        }
        success = true;
    }
    catch (const fs::filesystem_error &e) {
        std::cerr << clustro::Color::RED << "[STORAGE] Error creating directories: " << e.what() << clustro::Color::RESET << std::endl;
        success = false;
    }
}

Storage::Storage() : success(false), path("") {}

bool Storage::is_successful() const { 
    return success; 
}

// Save operations
bool Storage::save_user(User &u) {
    return save_item(u, "Users", u.get_id());
}

bool Storage::save_post(Post &p) {
    return save_item(p, "Posts", p.get_id());
}

bool Storage::save_comment(Comment &c) {
    return save_item(c, "Comments", c.get_id());
}

void Storage::save_follow(Follow &f) {
    save_item(f, "Follow", f.get_name());
}

void Storage::save_notification_block(const std::string &notification_data) {
    try {
        // Parse the notification data to get the ID
        std::istringstream stream(notification_data);
        std::string line;
        std::string notification_id;

        while (std::getline(stream, line)) {
            auto pos = line.find(":");
            if (pos == std::string::npos)
                continue;
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);
            if (key == "id") {
                notification_id = value;
                break;
            }
        }

        if (notification_id.empty()) {
            std::cerr << clustro::Color::RED << "[STORAGE] No ID found in notification data" << clustro::Color::RESET << std::endl;
            return;
        }

        std::string outputFilePath = this->path + "/Notifications/" + notification_id + ".txt";
        std::ofstream outputFile(outputFilePath);
        if (!outputFile.is_open()) {
            std::cerr << clustro::Color::RED << "[STORAGE] Failed to open notification file: " << outputFilePath << clustro::Color::RESET << std::endl;
            return;
        }

        outputFile << notification_data;
        std::cout << clustro::Color::GREEN << "[STORAGE] Notification saved to: " << outputFilePath << clustro::Color::RESET << std::endl;
    }
    catch (const std::exception &e) {
        std::cerr << clustro::Color::RED << "[STORAGE] Error saving notification: " << e.what() << clustro::Color::RESET << std::endl;
    }
}

// Load operations
clustro::LinkedList<Post> Storage::get_posts() {
    return load_items<Post>("Posts");
}

clustro::LinkedList<Comment> Storage::get_comments() {
    return load_items<Comment>("Comments");
}

clustro::LinkedList<User> Storage::get_users() {
    return load_items<User>("Users");
}

clustro::LinkedList<Follow> Storage::get_follow_data() {
    return load_items<Follow>("Follow");
}

clustro::LinkedList<User> Storage::get_users_indexed() {
    clustro::LinkedList<User> users = get_users();
    UserManager::index_from_linked_list(users);
    return users;
}

std::vector<std::string> Storage::load_notification_blocks() {
    std::vector<std::string> notifications;
    std::string dir_path = path + "/Notifications";

    if (!fs::exists(dir_path)) {
        std::cerr << clustro::Color::RED << "[STORAGE] Notifications directory does not exist: " << dir_path << clustro::Color::RESET << std::endl;
        return notifications;
    }

    for (const auto &entry : fs::directory_iterator(dir_path)) {
        if (entry.is_regular_file() && entry.path().extension() == ".txt") {
            std::string content = read_file_content<std::string>(entry.path());
            if (!content.empty()) {
                notifications.push_back(content);
            }
        }
    }
    return notifications;
}

// Delete operations
bool Storage::delete_user(const std::string &user_id) {
    return delete_item<User>("Users", user_id);
}

bool Storage::delete_post(const std::string &post_id) {
    return delete_item<Post>("Posts", post_id);
}

bool Storage::delete_comment(const std::string &comment_id) {
    return delete_item<Comment>("Comments", comment_id);
}

bool Storage::delete_follow(const std::string &follow_name) {
    return delete_item<Follow>("Follow", follow_name);
}

// Special operations
void Storage::rewrite_notification_blocks(const std::vector<std::string> &blocks) {
    try {
        // First, clear the existing notification files
        std::string dir_path = path + "/Notifications";
        if (fs::exists(dir_path)) {
            for (const auto &entry : fs::directory_iterator(dir_path)) {
                if (entry.is_regular_file() && entry.path().extension() == ".txt") {
                    fs::remove(entry.path());
                }
            }
        }

        // Now write all the blocks back
        for (const auto &block : blocks) {
            save_notification_block(block);
        }

        std::cout << clustro::Color::GREEN << "[STORAGE] All notification blocks rewritten" << clustro::Color::RESET << std::endl;
    }
    catch (const fs::filesystem_error &e) {
        std::cerr << clustro::Color::RED << "[STORAGE] Error rewriting notification blocks: " << e.what() << clustro::Color::RESET << std::endl;
    }
}

// Explicit template instantiations
template std::string Storage::read_file_content<std::string>(const fs::path &file_path);
template clustro::LinkedList<User> Storage::load_items<User>(const std::string &subdir);
template clustro::LinkedList<Post> Storage::load_items<Post>(const std::string &subdir);
template clustro::LinkedList<Comment> Storage::load_items<Comment>(const std::string &subdir);
template clustro::LinkedList<Follow> Storage::load_items<Follow>(const std::string &subdir);
template bool Storage::save_item<User>(User &item, const std::string &subdir, const std::string &id);
template bool Storage::save_item<Post>(Post &item, const std::string &subdir, const std::string &id);
template bool Storage::save_item<Comment>(Comment &item, const std::string &subdir, const std::string &id);
template bool Storage::save_item<Follow>(Follow &item, const std::string &subdir, const std::string &id);
template bool Storage::delete_item<User>(const std::string &subdir, const std::string &id);
template bool Storage::delete_item<Post>(const std::string &subdir, const std::string &id);
template bool Storage::delete_item<Comment>(const std::string &subdir, const std::string &id);
template bool Storage::delete_item<Follow>(const std::string &subdir, const std::string &id);