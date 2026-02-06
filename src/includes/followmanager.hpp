#pragma once
#ifndef FOLLOWMANAGER_HPP
#define FOLLOWMANAGER_HPP

#include <iostream>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <random>
#include "utils/LinkedList.hpp"
#include "user.hpp"
#include "follow.hpp"
#include "storage.hpp"

// Forward declarations

struct FollowNode
{
    std::string user_id;
    clustro::LinkedList<std::string> following;  // Store user IDs instead of pointers
    clustro::LinkedList<std::string> followers;  // Track who follows this user
    
    FollowNode(const std::string& id) : user_id(id) {}
};

class FollowManager
{
private:
    std::unordered_map<std::string, FollowNode *> all_users;
    UserManager* user_manager;
    Storage* storage; // Added storage pointer

    // Helper function to get or create a user node
    FollowNode *get_or_create_user_node(const std::string &user_id);
    
    // Helper function to check if a user follows another
    bool is_following(const std::string& follower_id, const std::string& followee_id);
    
    // Helper function to remove an item from a LinkedList
    template<typename T>
    bool remove_from_list(clustro::LinkedList<T>& list, const T& item);
    
    // Load existing follows from storage
    void load_follows();

public:
    // Constructor with UserManager and Storage pointers
    FollowManager(UserManager* manager, Storage* storage_ptr);
    
    // Add a follow relationship
    bool add_follower(const std::string& follower_id, const std::string& followee_id);
    
    // Remove a follow relationship
    bool remove_follower(const std::string& follower_id, const std::string& followee_id);
    
    // Get all users that a user is following
    clustro::LinkedList<std::string> get_following(const std::string& user_id);
    
    // Get all followers of a user
    clustro::LinkedList<std::string> get_followers(const std::string& user_id);
    
    // Get suggestions for a user
    clustro::LinkedList<User *> get_suggestions_for_user(const std::string &user_id, int max_suggestions = 10);
    
    // Destructor to clean up memory
    ~FollowManager();
};

#endif