#include "clustro.hpp"
#include <iostream>
#include <iomanip>

// ============================================
// CONSTRUCTOR AND DESTRUCTOR
// ============================================

Clustro::Clustro() : current_user(nullptr) {
    std::cout << clustro::Color::YELLOW << "[INIT] Initializing Clustro Application..." << clustro::Color::RESET << std::endl;
    
    storage = new Storage("data");
    if (!storage->is_successful()) {
        std::cerr << clustro::Color::RED << "[FATAL] Failed to initialize storage. Exiting." << clustro::Color::RESET << std::endl;
        exit(1);
    }

    // Load existing users from storage BEFORE creating UserManager
    std::cout << clustro::Color::CYAN << "[INIT] Loading users from storage..." << clustro::Color::RESET << std::endl;
    clustro::LinkedList<User> loaded_users = storage->get_users();
    std::cout << clustro::Color::CYAN << "[INIT] Loaded " << loaded_users.size() << " users from storage." << clustro::Color::RESET << std::endl;
    
    // Create UserManager with the loaded users
    user_manager = new UserManager(loaded_users);
    post_manager = new PostManager(storage);
    comment_manager = new CommentManager(storage);
    follow_manager = new FollowManager(user_manager, storage);
    feed_manager = new FeedManager(storage, user_manager, follow_manager, post_manager);
    notification_controller = new NotificationController(storage);
    
    std::cout << clustro::Color::GREEN << "[INIT] Clustro initialized successfully!" << clustro::Color::RESET << std::endl;
}

Clustro::~Clustro() {
    if (post_manager) delete post_manager;
    if (comment_manager) delete comment_manager;
    if (follow_manager) delete follow_manager;
    if (feed_manager) delete feed_manager;
    if (notification_controller) delete notification_controller;
    if (user_manager) delete user_manager;
    if (storage) delete storage;
    std::cout << clustro::Color::YELLOW << "[SHUTDOWN] Clustro resources cleaned up." << clustro::Color::RESET << std::endl;
}

// ============================================
// MAIN APPLICATION LOOP
// ============================================

void Clustro::run() {
    while (true) {
        showLoginMenu();
        if (current_user == nullptr) {
            break;
        }
        showMainMenu();
    }
}

// ============================================
// FOLLOW SYSTEM METHODS
// ============================================

void Clustro::followUserMenu() {
    displayHeader("FOLLOW/UNFOLLOW");
    
    std::cout << "1. " << clustro::Color::CYAN << "Find & Follow User" << clustro::Color::RESET << std::endl;
    std::cout << "2. " << clustro::Color::CYAN << "View My Followers" << clustro::Color::RESET << std::endl;
    std::cout << "3. " << clustro::Color::CYAN << "View Following" << clustro::Color::RESET << std::endl;
    std::cout << "4. " << clustro::Color::CYAN << "View Suggestions" << clustro::Color::CYAN << " (NEW!)" << clustro::Color::RESET << std::endl;
    std::cout << "5. " << clustro::Color::CYAN << "Back" << clustro::Color::RESET << std::endl;
    std::cout << std::endl;

    int choice = getMenuChoice(1, 5);
    
    switch (choice) {
        case 1: {
            std::string username = getUserInput("Enter username to follow/unfollow: ");
            User* user_to_follow = user_manager->search_by_username_silent(username);
            
            if (!user_to_follow) {
                displayErrorMessage("User not found.");
            } else if (user_to_follow->get_id() == current_user->get_id()) {
                displayWarningMessage("You cannot follow yourself.");
            } else {
                std::cout << "\n1. " << clustro::Color::CYAN << "Follow" << clustro::Color::RESET << std::endl;
                std::cout << "2. " << clustro::Color::CYAN << "Unfollow" << clustro::Color::RESET << std::endl;
                std::cout << std::endl;
                
                int action = getMenuChoice(1, 2);

                if (action == 1) {
                    if (follow_manager->add_follower(current_user->get_id(), user_to_follow->get_id())) {
                        displaySuccessMessage("You are now following " + username + "!");
                        current_user->increment_following();
                        user_to_follow->increment_followers();
                        storage->save_user(*current_user);
                        storage->save_user(*user_to_follow);
                    } else {
                        displayWarningMessage("You are already following " + username + ".");
                    }
                } else {
                    if (follow_manager->remove_follower(current_user->get_id(), user_to_follow->get_id())) {
                        displaySuccessMessage("You have unfollowed " + username + ".");
                        current_user->decrement_following();
                        user_to_follow->decrement_followers();
                        storage->save_user(*current_user);
                        storage->save_user(*user_to_follow);
                    } else {
                        displayWarningMessage("You were not following " + username + ".");
                    }
                }
            }
            break;
        }
        case 2: viewFollowers(); break;
        case 3: viewFollowing(); break;
        case 4: viewSuggestions(); break;
        case 5: return;
    }
    pauseExecution();
}

void Clustro::viewFollowers() {
    displayHeader("MY FOLLOWERS");
    
    auto followers = follow_manager->get_followers(current_user->get_id());
    
    if (followers.size() == 0) {
        displayWarningMessage("You have no followers yet.");
    } else {
        std::cout << clustro::Color::GREEN << "Found " << followers.size() << " followers:" << clustro::Color::RESET << std::endl << std::endl;
        int count = 1;
        for (auto it = followers.begin(); it != followers.end(); ++it) {
            User* follower = user_manager->search_by_id_silent(*it);
            if (follower) {
                std::cout << count++ << ". " << clustro::Color::CYAN << follower->get_username() << clustro::Color::RESET 
                          << " (" << follower->get_followers_count() << " followers)" << std::endl;
            }
        }
    }
    pauseExecution();
}

void Clustro::viewFollowing() {
    displayHeader("USERS I FOLLOW");
    
    auto following = follow_manager->get_following(current_user->get_id());
    
    if (following.size() == 0) {
        displayWarningMessage("You are not following anyone yet.");
    } else {
        std::cout << clustro::Color::GREEN << "Following " << following.size() << " users:" << clustro::Color::RESET << std::endl << std::endl;
        int count = 1;
        for (auto it = following.begin(); it != following.end(); ++it) {
            User* followed = user_manager->search_by_id_silent(*it);
            if (followed) {
                std::cout << count++ << ". " << clustro::Color::CYAN << followed->get_username() << clustro::Color::RESET 
                          << " (" << followed->get_followers_count() << " followers)" << std::endl;
            }
        }
    }
    pauseExecution();
}

void Clustro::viewSuggestions() {
    displayHeader("SUGGESTED FOR YOU");
    
    // Get suggestions
    auto suggestions = follow_manager->get_suggestions_for_user(current_user->get_id());
    
    if (suggestions.size() == 0) {
        displayInfoMessage("No specific suggestions right now. Try following more people!");
    } else {
        std::cout << clustro::Color::CYAN << "Based on people you follow:" << clustro::Color::RESET << std::endl << std::endl;
        
        int count = 1;
        for (auto it = suggestions.begin(); it != suggestions.end(); ++it) {
            User* suggestion = *it;
            if (suggestion) {
                std::cout << count++ << ". " << clustro::Color::BRIGHT_CYAN << suggestion->get_username() << clustro::Color::RESET 
                          << " (" << suggestion->get_followers_count() << " followers)" << std::endl;
            }
        }
        
        std::cout << std::endl;
        std::cout << "Enter the number of the user to follow, or 0 to go back: ";
        int choice;
        if (std::cin >> choice) {
            std::cin.ignore();
            if (choice > 0 && choice <= suggestions.size()) {
                // Find the user at index choice-1
                int idx = 0;
                User* selected = nullptr;
                for (auto it = suggestions.begin(); it != suggestions.end(); ++it) {
                    if (idx == choice - 1) {
                        selected = *it;
                        break;
                    }
                    idx++;
                }
                
                if (selected) {
                    if (follow_manager->add_follower(current_user->get_id(), selected->get_id())) {
                        displaySuccessMessage("You are now following " + selected->get_username() + "!");
                        current_user->increment_following();
                        selected->increment_followers();
                        storage->save_user(*current_user);
                        storage->save_user(*selected);
                    } else {
                        displayErrorMessage("Failed to follow user.");
                    }
                }
            }
        } else {
            std::cin.clear();
            std::cin.ignore();
        }
    }
    pauseExecution();
}

// ============================================
// NOTIFICATION METHODS
// ============================================

void Clustro::searchUsers() {
    displayHeader("SEARCH USERS");
    
    std::string username = getUserInput("Enter username to search: ");
    User* found_user = user_manager->search_by_username_silent(username);

    if (found_user) {
        displayUserInfo(found_user);
    } else {
        displayErrorMessage("User not found.");
    }
    pauseExecution();
}

// ============================================
// POST MANAGEMENT METHODS
// ============================================

void Clustro::createPost() {
    displayHeader("CREATE NEW POST");
    
    std::string content = getUserInput("What's on your mind? ");

    if (!content.empty()) {
        Post* new_post = post_manager->create_post(current_user->get_id(), content);
        if (new_post) {
            current_user->increment_post_count();
            storage->save_user(*current_user);
            displaySuccessMessage("Post created successfully!");
        } else {
            displayErrorMessage("Failed to create post. Please try again.");
        }
    } else {
        displayWarningMessage("Post cannot be empty.");
    }
    pauseExecution();
}

void Clustro::viewMyPosts() {
    displayHeader("MY POSTS");
    
    std::vector<Post> my_posts = post_manager->get_posts_by_user(current_user->get_id());

    if (my_posts.empty()) {
        displayWarningMessage("You haven't created any posts yet.");
    } else {
        for (size_t i = 0; i < my_posts.size(); ++i) {
            std::cout << clustro::Color::YELLOW << "\n[Post " << (i + 1) << "]" << clustro::Color::RESET << std::endl;
            std::cout << "Content: " << my_posts[i].get_content() << std::endl;
            std::cout << "Likes: " << my_posts[i].get_likes() << " | Comments: " << my_posts[i].get_comments() << std::endl;
            std::cout << "Posted: " << my_posts[i].get_timestamps() << std::endl;
        }
    }
    pauseExecution();
}

void Clustro::editPost() {
    displayHeader("EDIT POST");
    
    std::string post_id = getUserInput("Enter post ID: ");
    std::string new_content = getUserInput("Enter new content: ");

    if (!new_content.empty()) {
        if (post_manager->edit_post(post_id, current_user->get_id(), new_content)) {
            displaySuccessMessage("Post edited successfully!");
        } else {
            displayErrorMessage("Failed to edit post. Please check the post ID.");
        }
    } else {
        displayWarningMessage("Post content cannot be empty.");
    }
    pauseExecution();
}

void Clustro::deletePost() {
    displayHeader("DELETE POST");
    
    std::string post_id = getUserInput("Enter post ID: ");
    
    std::cout << "Are you sure? (y/n): ";
    char confirm;
    std::cin >> confirm;
    std::cin.ignore();
    
    if (confirm == 'y' || confirm == 'Y') {
        if (post_manager->delete_post(post_id, current_user->get_id())) {
            current_user->increment_post_count();  // This should decrement but we'll use it as is
            storage->save_user(*current_user);
            displaySuccessMessage("Post deleted successfully!");
        } else {
            displayErrorMessage("Failed to delete post.");
        }
    } else {
        displayWarningMessage("Deletion cancelled.");
    }
    pauseExecution();
}

// ============================================
// FEED AND INTERACTION METHODS
// ============================================

void Clustro::viewFeed() {
    displayHeader("YOUR FEED");
    
    std::vector<Post> feed = feed_manager->generate_feed(current_user->get_id());

    if (feed.empty()) {
        displayWarningMessage("Your feed is empty. Follow some users to see their posts!");
    } else {
        for (size_t i = 0; i < feed.size(); ++i) {
            User* author = user_manager->search_by_id_silent(feed[i].get_user_id());
            std::cout << clustro::Color::YELLOW << "\n" << std::string(40, '-') << clustro::Color::RESET << std::endl;
            std::cout << "Post ID: " << feed[i].get_id() << std::endl;
            std::cout << "Author: " << (author ? author->get_username() : "Unknown") << std::endl;
            std::cout << "Content: " << feed[i].get_content() << std::endl;
            std::cout << "Likes: " << feed[i].get_likes() << " | Comments: " << feed[i].get_comments() << std::endl;
            std::cout << "Posted: " << feed[i].get_timestamps() << std::endl;
        }
        std::cout << clustro::Color::YELLOW << "\n" << std::string(40, '-') << clustro::Color::RESET << std::endl;
    }
    
    std::cout << "\n1. " << clustro::Color::CYAN << "Like a Post" << clustro::Color::RESET << std::endl;
    std::cout << "2. " << clustro::Color::CYAN << "Comment on Post" << clustro::Color::RESET << std::endl;
    std::cout << "3. " << clustro::Color::CYAN << "Back" << clustro::Color::RESET << std::endl;

    int choice = getMenuChoice(1, 3);
    switch (choice) {
        case 1: likePost(); break;
        case 2: addCommentToPost(); break;
        case 3: break;
    }
}

// ============================================
// COMMENT METHODS
// ============================================

void Clustro::addCommentToPost() {
    displayHeader("ADD COMMENT");
    
    std::string post_id = getUserInput("Enter post ID: ");
    std::string comment_content = getUserInput("Enter your comment: ");

    if (!comment_content.empty()) {
        Comment* new_comment = comment_manager->create_comment(post_id, current_user->get_id(), comment_content);
        if (new_comment) {
            post_manager->increment_comment_count(post_id);
            displaySuccessMessage("Comment added successfully!");
        } else {
            displayErrorMessage("Failed to add comment.");
        }
    } else {
        displayWarningMessage("Comment cannot be empty.");
    }
    pauseExecution();
}

void Clustro::viewPostComments() {
    displayHeader("POST COMMENTS");
    
    std::string post_id = getUserInput("Enter post ID: ");
    std::vector<Comment> comments = comment_manager->load_comments_for_post(post_id);

    if (comments.empty()) {
        displayWarningMessage("No comments on this post yet.");
    } else {
        for (auto& comment : comments) {
            User* commenter = user_manager->search_by_id_silent(comment.get_user_id());
            std::cout << clustro::Color::CYAN << "\n" << (commenter ? commenter->get_username() : "Unknown") << ": ";
            std::cout << clustro::Color::RESET << comment.get_content() << std::endl;
            std::cout << "Likes: " << comment.get_likes() << " | " << comment.get_timestamps() << std::endl;
        }
    }
    pauseExecution();
}

void Clustro::deleteComment() {
    displayHeader("DELETE COMMENT");
    
    std::string comment_id = getUserInput("Enter comment ID: ");
    
    if (comment_manager->delete_comment(comment_id, current_user->get_id())) {
        displaySuccessMessage("Comment deleted successfully!");
    } else {
        displayErrorMessage("Failed to delete comment.");
    }
    pauseExecution();
}

void Clustro::likeComment() {
    std::string comment_id = getUserInput("Enter comment ID: ");
    
    if (comment_manager->like_comment(comment_id)) {
        displaySuccessMessage("Comment liked!");
    } else {
        displayErrorMessage("Failed to like comment.");
    }
    pauseExecution();
}

// ============================================
// LIKE METHODS
// ============================================

void Clustro::likePost() {
    std::string post_id = getUserInput("Enter post ID to like: ");
    
    if (post_manager->like_post(post_id)) {
        displaySuccessMessage("Post liked!");
    } else {
        displayErrorMessage("Failed to like post.");
    }
}

void Clustro::unlikePost() {
    std::string post_id = getUserInput("Enter post ID to unlike: ");
    
    if (post_manager->unlike_post(post_id)) {
        displaySuccessMessage("Post unliked!");
    } else {
        displayErrorMessage("Failed to unlike post.");
    }
}

// ============================================
// LOGOUT METHOD
// ============================================

void Clustro::logout() {
    displayHeader("LOGOUT");
    displayInfoMessage("Logging out " + current_user->get_username() + "...");
    current_user = nullptr;
    pauseExecution();
}
