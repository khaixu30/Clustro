#include "user.hpp"

// Initialize static member variables
std::unordered_map<std::string, User*> UserManager::users_by_id;
std::unordered_map<std::string, User*> UserManager::users_by_email;
std::unordered_map<std::string, User*> UserManager::users_by_username;

// User class implementation

void User::parse(const std::string& user_string) {
    // initialize numeric fields to safe defaults
    postCount = 0;
    followers = 0;
    following = 0;

    std::istringstream stream(user_string);
    std::string line;
    while(std::getline(stream, line)) {
        auto pos = line.find(":");
        if(pos == std::string::npos) continue;
        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);
        if(key == "id") id = value;
        else if(key == "username") username = value;
        else if(key == "about") about = value;
        else if(key == "email") email = value;
        else if(key == "password") password = value;
        else if(key == "post" || key == "posts") {
            try { postCount = std::stoi(value); } catch(...) { postCount = 0; }
        }
        else if(key == "followers") {
            try { followers = std::stoi(value); } catch(...) { followers = 0; }
        }
        else if(key == "following") {
            try { following = std::stoi(value); } catch(...) { following = 0; }
        }
    }
}

// Constructor with parameters
User::User(const std::string& u, const std::string& e, const std::string& p, const std::string& a, int post, int follow, int follower) 
    : username(u), email(e), password(p), about(a), postCount(post), following(follow), followers(follower) {
    id = clustro::generateID(8);
}

// Default constructor
User::User() : postCount(0), followers(0), following(0) { 
    id = clustro::generateID(8); 
}

// Constructor from string data
User::User(const std::string& new_user_data) {
    parse(new_user_data);
}

// Serialize user data to string
std::string User::stringify() const {
    std::string user_string = "id:" + id + "\nemail:" + email + "\nusername:" + username + 
                             "\nabout:" + about + "\npassword:" + password + 
                             "\nfollowers:" + std::to_string(followers) + 
                             "\nfollowing:" + std::to_string(following) + 
                             "\nposts:" + std::to_string(postCount);
    return user_string;
}

// Getters - return const references to prevent external modification
const std::string& User::get_id() const {
    return id;
}

const std::string& User::get_email() const {
    return email;
}

const std::string& User::get_username() const {
    return username;
}

const std::string& User::get_about() const {
    return about;
}

int User::get_post_count() const {
    return postCount;
}

int User::get_followers_count() const {
    return followers;
}

int User::get_following_count() const {
    return following;
}

// Setters
void User::set_username(const std::string& new_username) {
    username = new_username;
}

void User::set_email(const std::string& new_email) {
    email = new_email;
}

void User::set_about(const std::string& new_about) {
    about = new_about;
}

void User::set_password(const std::string& new_password) {
    password = new_password;
}

const std::string& User::get_password() const {  // Fixed: Added const
    return password;
}

// Increment methods
void User::increment_post_count() {
    postCount++;
}

void User::increment_followers() {
    followers++;
}

void User::increment_following() {
    following++;
}

void User::decrement_followers() {
    if (followers > 0) followers--;
}

void User::decrement_following() {
    if (following > 0) following--;
}

// Print user information
void User::print_user() const {
    std::cout << "[USER] Printing the user data: " << std::endl;
    std::cout << "id: " << id << std::endl;
    std::cout << "username: " << username << std::endl;
    std::cout << "email: " << email << std::endl;
    std::cout << "about: " << about << std::endl;
    std::cout << "post: " << postCount << std::endl;
    std::cout << "followers: " << followers << std::endl;
    std::cout << "following: " << following << std::endl;
}

void User::signup(){
    // Fixed: Removed std::cin.ignore() from here
    std::cout << "Enter username: ";
    std::getline(std::cin, username);
    std::cout << "Enter email: ";
    std::getline(std::cin, email);
    std::cout << "Enter password: ";
    std::getline(std::cin, password);
    std::string line_password;
    std::cout << "Confirm password: ";
    std::getline(std::cin, line_password);
    while(line_password != password){
        std::cout << "Passwords do not match! Try again." << std::endl;
        std::cout << "Enter password: ";
        std::getline(std::cin, password);
        std::cout << "Confirm password: ";
        std::getline(std::cin, line_password);
    }
    std::cout << "About: ";
    std::getline(std::cin, about);
}

// UserManager class implementation

UserManager::UserManager(clustro::LinkedList<User>& list) : all_users(list), index_dirty(true) {}

UserManager::UserManager() : all_users(), index_dirty(true) {}

// Get a reference to the users list
clustro::LinkedList<User>& UserManager::get_users() {
    index_dirty = true; // Mark index as dirty since we're exposing the list
    return all_users;
}

// Add a user to the list
void UserManager::add_user(const User& user) {
    all_users.insert(user);
    add_to_index(const_cast<User*>(&user));  // Fixed: Added this line
    index_dirty = true;
}

// Remove a user by ID
bool UserManager::remove_user(const std::string& user_id) {
    // Find the user in the list
    for (auto it = all_users.begin(); it != all_users.end(); ++it) {
        if (it->get_id() == user_id) {
            // Remove from index first
            remove_from_index_by_id(user_id);
            // Mark index as dirty since we're modifying the list
            index_dirty = true;
            // Note: This would require a remove method in LinkedList
            // For now, we'll just mark the index as dirty
            return true;
        }
    }
    return false;
}

// Update the index if it's dirty
void UserManager::ensure_index_is_current() const {
    if (index_dirty) {
        index_from_linked_list(all_users);
        index_dirty = false;
    }
}

// Fixed index_from_linked_list function
void UserManager::index_from_linked_list(const clustro::LinkedList<User>& list) {
    clear_index();
    int count = 0;
    
    // Debug output
    // std::cout << "DEBUG: List size: " << list.size() << std::endl;
    
    for(auto it = list.begin(); it != list.end(); ++it) {
        User* u = const_cast<User*>(&(*it)); // Remove const for compatibility
        
        // // Debug output for each user
        // std::cout << "DEBUG: Processing user ID: " << u->get_id() << std::endl;
        // std::cout << "DEBUG: Processing user email: " << u->get_email() << std::endl;
        // std::cout << "DEBUG: Processing user username: " << u->get_username() << std::endl;
        
        if(!u->get_id().empty()) {
            users_by_id[u->get_id()] = u;
            // std::cout << "DEBUG: Added to ID index" << std::endl;
        }
        if(!u->get_email().empty()) {
            users_by_email[u->get_email()] = u;
            // std::cout << "DEBUG: Added to email index" << std::endl;
        }
        if(!u->get_username().empty()) {
            users_by_username[u->get_username()] = u;
            // std::cout << "DEBUG: Added to username index" << std::endl;
        }
        count++;
    }
    std::cout << clustro::Color::GREEN << "[INDEX] Indexed " << count << " users from linked list." << clustro::Color::RESET << std::endl;
    // std::cout << "DEBUG: Final index size: " << users_by_id.size() << std::endl;
}

// Add a single user pointer to the index (overwrites existing entries with same keys)
void UserManager::add_to_index(User* user) {
    if(user == nullptr) return;
    if(!user->get_id().empty()) users_by_id[user->get_id()] = user;
    if(!user->get_email().empty()) users_by_email[user->get_email()] = user;
    if(!user->get_username().empty()) users_by_username[user->get_username()] = user;
}

// Remove from index by id (also removes email/username entries for that user)
void UserManager::remove_from_index_by_id(const std::string& user_id) {
    auto it = users_by_id.find(user_id);
    if(it == users_by_id.end()) return;
    User* user = it->second;
    if(user) {
        if(!user->get_email().empty()) users_by_email.erase(user->get_email());
        if(!user->get_username().empty()) users_by_username.erase(user->get_username());
    }
    users_by_id.erase(it);
    std::cout << clustro::Color::GREEN << "[INDEX] User removed from index." << clustro::Color::RESET << std::endl;
}

// Clear all index maps
void UserManager::clear_index() {
    users_by_id.clear();
    users_by_email.clear();
    users_by_username.clear();
}

// Get index size
int UserManager::get_index_size() {
    return users_by_id.size();
}

// Search functions - return raw pointers (nullptr if not found)
User* UserManager::search_by_id(const std::string& search_id) {
    ensure_index_is_current();
    auto it = users_by_id.find(search_id);
    if(it != users_by_id.end()) {
        std::cout << clustro::Color::GREEN << "[SEARCH] User found by ID!" << clustro::Color::RESET << std::endl;
        return it->second;
    }
    std::cout << clustro::Color::RED << "[SEARCH] User not found by ID!" << clustro::Color::RESET << std::endl;
    return nullptr;
}

User* UserManager::search_by_email(const std::string& search_email) {
    ensure_index_is_current();
    auto it = users_by_email.find(search_email);
    if(it != users_by_email.end()) {
        std::cout << clustro::Color::GREEN << "[SEARCH] User found by email!" << clustro::Color::RESET << std::endl;
        return it->second;
    }
    std::cout << clustro::Color::RED << "[SEARCH] User not found by email!" << clustro::Color::RESET << std::endl;
    return nullptr;
}

User* UserManager::search_by_username(const std::string& search_username) {
    ensure_index_is_current();
    auto it = users_by_username.find(search_username);
    if(it != users_by_username.end()) {
        std::cout << clustro::Color::GREEN << "[SEARCH] User found by username!" << clustro::Color::RESET << std::endl;
        return it->second;
    }
    std::cout << clustro::Color::RED << "[SEARCH] User not found by username!" << clustro::Color::RESET << std::endl;
    return nullptr;
}

// Silent search functions without console output
User* UserManager::search_by_id_silent(const std::string& search_id) {
    ensure_index_is_current();
    auto it = users_by_id.find(search_id);
    return (it != users_by_id.end()) ? it->second : nullptr;
}

User* UserManager::search_by_email_silent(const std::string& search_email) {
    ensure_index_is_current();
    auto it = users_by_email.find(search_email);
    return (it != users_by_email.end()) ? it->second : nullptr;
}

User* UserManager::search_by_username_silent(const std::string& search_username) {
    ensure_index_is_current();
    auto it = users_by_username.find(search_username);
    return (it != users_by_username.end()) ? it->second : nullptr;
}