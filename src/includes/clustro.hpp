#pragma once
// CLUSTRO_HPP
#ifndef CLUSTRO_HPP
#define CLUSTRO_HPP

#include "user.hpp"
#include "follow.hpp"
#include "utils/utils.hpp"
#include "storage.hpp"
#include "post.hpp"
#include "comment.hpp"
#include "notification.hpp"
#include "followmanager.hpp"
#include "feedmanager.hpp"
#include <string>
#include <limits>
#include <vector>

/*
    CLUSTRO CLASS (Main Application Controller)
    ============================================
    This is the main class that orchestrates all operations in the social media application.
    It serves as the entry point for user interactions and manages the flow of the entire application.
    
    Features:
    - User Authentication (Login/Signup)
    - Post Management (Create, View, Like, Delete)
    - Feed Management (Personalized feed based on following)
    - User Follow System (Follow/Unfollow users)
    - Comment System (Add, View, Edit comments on posts)
    - Notification System (User notifications)
    - User Profile Management
*/


class Clustro {
private:
     // ========== User/Follow Operations ==========
    void viewProfile();
    void viewOtherUserProfile();
    void followUserMenu();
    void viewFollowers();
    void viewFollowing();
    void viewSuggestions(); // Added suggestion viewer
    void searchUsers();

// ========== Post Operations ==========
    void createPost();
    void viewMyPosts();
    void deletePost();
    void likePost();
    void unlikePost();
    void editPost();
    
    // ========== Feed Operations ==========
    void viewFeed();
    void viewUserFeed(const std::string& user_id);
    void interactWithPost();
    
    // ========== Comment Operations ==========
    void addCommentToPost();
    void viewPostComments();
    void deleteComment();
    void likeComment();

public:
    // ========== Constructor & Destructor ==========
    Clustro();
    ~Clustro();
    
    // ========== Main Application Loop ==========
    void run();
};
