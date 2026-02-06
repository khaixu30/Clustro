#pragma once
#ifndef USER_HPP
#define USER_HPP

/*
    USER_HPP:
        Header file for User class.
        Most of the tasks related to the users are handled here!
*/

/*
    LAST COMMIT:
    ============
    (Isko hr commit se phalay edit krna hai for better communications)
    > User class created.
    > random id generator.
    > Stringify function created
*/

#include <iostream>
#include <string>
#include <sstream>
#include <unordered_map>
#include "utils/utils.hpp"
#include "utils/LinkedList.hpp"

class UserManager;

class User {
private:
    std::string id; // Unique key to identify users
    std::string username;   // username also unique to identify users
    std::string email;  // email is unique id to find account while login
    std::string password;   // simple text password for now! hashing function will be added further
    std::string about;  // 
    int postCount;
    int followers;
    int following;

    friend class UserManager;
    
    void parse(const std::string& user_string);

public:
    // Constructor with parameters
    User(const std::string& u, const std::string& e, const std::string& p, const std::string& a, int post, int follow, int follower);
    
    // Default constructor
    User();
    
    // Constructor from string data
    User(const std::string& new_user_data);

    // Serialize user data to string
    std::string stringify() const;

    // Getters - return const references to prevent external modification
    const std::string& get_id() const;
    const std::string& get_email() const;
    const std::string& get_username() const;
    const std::string& get_about() const;
    int get_post_count() const;
    int get_followers_count() const;
    int get_following_count() const;

    // Setters
    void set_username(const std::string& new_username);
    void set_email(const std::string& new_email);
    void set_about(const std::string& new_about);
    void set_password(const std::string& new_password);
    const std::string& get_password() const;

    // Increment methods
    void increment_post_count();
    void increment_followers();
    void increment_following();
    void decrement_followers();
    void decrement_following();

    // Print user information
    void print_user() const;
    
    // User signup
    void signup();
};

class UserManager {
private:
    clustro::LinkedList<User> all_users;
    
    // Flag to track if the index is up-to-date with the linked list
    mutable bool index_dirty;
    
    // Update the index if it's dirty
    void ensure_index_is_current() const;

public:
    UserManager(clustro::LinkedList<User>& list);
    UserManager();
    
    // Get a reference to the users list
    clustro::LinkedList<User>& get_users();
    
    // Add a user to the list
    void add_user(const User& user);
    
    // Remove a user by ID
    bool remove_user(const std::string& user_id);

    /* ------------------- LinkedList -> Hash-table index & search -------------------
     *
     * - Index is built from an in-memory clustro::LinkedList<User>.
     * - The maps store raw User* pointers that point into the list nodes.
     * - Pointers remain valid while the linked list is not modified/destroyed.
     * - Lookups are O(1) average.
     * 
     */
    
    // Static maps for indexing
    static std::unordered_map<std::string, User*> users_by_id;
    static std::unordered_map<std::string, User*> users_by_email;
    static std::unordered_map<std::string, User*> users_by_username;

    // Build the index from an existing linked list of users
    static void index_from_linked_list(const clustro::LinkedList<User>& list);

    // Add a single user pointer to the index (overwrites existing entries with same keys)
    static void add_to_index(User* user);

    // Remove from index by id (also removes email/username entries for that user)
    static void remove_from_index_by_id(const std::string& user_id);

    // Clear all index maps
    static void clear_index();

    // Get index size
    static int get_index_size();

    // Search functions - return raw pointers (nullptr if not found)
    User* search_by_id(const std::string& search_id);
    User* search_by_email(const std::string& search_email);
    User* search_by_username(const std::string& search_username);
    
    // Silent search functions without console output
    User* search_by_id_silent(const std::string& search_id);
    User* search_by_email_silent(const std::string& search_email);
    User* search_by_username_silent(const std::string& search_username);
};

#endif