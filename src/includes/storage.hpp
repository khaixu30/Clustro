#pragma once
#ifndef STORAGE_HPP
#define STORAGE_HPP

#include <iostream>
#include <filesystem>
#include <fstream>
#include <vector>
#include <memory>
#include <string>

// Include LinkedList before using it
#include "utils/LinkedList.hpp"

// Forward declarations
class User;
class Post;
class Comment;
class Follow;
class Notification;

namespace fs = std::filesystem;

class Storage {
private:
    std::string path;
    bool success;

    // Helper function to safely read file content
    template <typename T>
    std::string read_file_content(const fs::path &file_path);

    // Generic function to load items from a directory
    template <typename T>
    clustro::LinkedList<T> load_items(const std::string &subdir);

    // Generic function to save an item to a file
    template <typename T>
    bool save_item(T &item, const std::string &subdir, const std::string &id);

    // Function to delete an item
    template <typename T>
    bool delete_item(const std::string &subdir, const std::string &id);

public:
    Storage(const std::string &path);
    Storage();
    bool is_successful() const;

    // Save operations
    bool save_user(User &u);
    bool save_post(Post &p);
    bool save_comment(Comment &c);
    void save_follow(Follow &f);
    void save_notification_block(const std::string &notification_data);

    // Load operations
    clustro::LinkedList<Post> get_posts();
    clustro::LinkedList<Comment> get_comments();
    clustro::LinkedList<User> get_users();
    clustro::LinkedList<Follow> get_follow_data();
    clustro::LinkedList<User> get_users_indexed();
    std::vector<std::string> load_notification_blocks();

    // Delete operations
    bool delete_user(const std::string &user_id);
    bool delete_post(const std::string &post_id);
    bool delete_comment(const std::string &comment_id);
    bool delete_follow(const std::string &follow_name);

    // Special operations
    void rewrite_notification_blocks(const std::vector<std::string> &blocks);
};

#endif