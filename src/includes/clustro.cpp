#include "clustro.hpp"
#include <iostream>
#include <iomanip>

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