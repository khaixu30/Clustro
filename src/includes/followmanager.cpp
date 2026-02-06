#include "followmanager.hpp"

// Helper function to get or create a user node
FollowNode *FollowManager::get_or_create_user_node(const std::string &user_id)
{
    if (all_users.find(user_id) == all_users.end())
    {
        all_users[user_id] = new FollowNode(user_id);
    }
    return all_users[user_id];
}

// Helper function to check if a user follows another
bool FollowManager::is_following(const std::string& follower_id, const std::string& followee_id)
{
    auto follower_it = all_users.find(follower_id);
    if (follower_it == all_users.end()) {
        return false;
    }
    
    return follower_it->second->following.contains(followee_id);
}

// Helper function to remove an item from a LinkedList
template<typename T>
bool FollowManager::remove_from_list(clustro::LinkedList<T>& list, const T& item)
{
    // Create a new list without the item to remove
    clustro::LinkedList<T> new_list;
    bool found = false;
    
    for (auto it = list.begin(); it != list.end(); ++it) {
        if (*it == item) {
            found = true;
            continue; // Skip this item
        }
        new_list.insert(*it);
    }
    
    if (found) {
        list = std::move(new_list);
    }
    
    return found;
}

// Load existing follows from storage
void FollowManager::load_follows()
{
    if (!storage) return;
    
    auto follows = storage->get_follow_data();
    std::cout << clustro::Color::CYAN << "[INIT] Loading " << follows.size() << " follow relationships..." << clustro::Color::RESET << std::endl;
    
    for (auto it = follows.begin(); it != follows.end(); ++it) {
        Follow follow = *it;
        std::string follower_id = follow.get_follower_id();
        std::string followee_id = follow.get_followee_id();
        
        // Get or create nodes
        FollowNode *follower_node = get_or_create_user_node(follower_id);
        FollowNode *followee_node = get_or_create_user_node(followee_id);
        
        // Add relationship to memory (without saving again)
        follower_node->following.insert(followee_id);
        followee_node->followers.insert(follower_id);
    }
}

// Constructor with UserManager and Storage pointers
FollowManager::FollowManager(UserManager* manager, Storage* storage_ptr) : user_manager(manager), storage(storage_ptr) {
    load_follows();
}

// Add a follow relationship
bool FollowManager::add_follower(const std::string& follower_id, const std::string& followee_id)
{
    // Can't follow yourself
    if (follower_id == followee_id) {
        return false;
    }
    
    // Check if already following
    if (is_following(follower_id, followee_id)) {
        return false;
    }
    
    // Get or create nodes
    FollowNode *follower_node = get_or_create_user_node(follower_id);
    FollowNode *followee_node = get_or_create_user_node(followee_id);
    
    // Add the follow relationship
    follower_node->following.insert(followee_id);
    followee_node->followers.insert(follower_id);
    
    // Persist to storage
    if (storage && user_manager) {
        User* follower = user_manager->search_by_id_silent(follower_id);
        User* followee = user_manager->search_by_id_silent(followee_id);
        
        if (follower && followee) {
            Follow new_follow(*followee, *follower);
            storage->save_follow(new_follow);
        }
    }
    
    return true;
}

// Remove a follow relationship
bool FollowManager::remove_follower(const std::string& follower_id, const std::string& followee_id)
{
    if (!is_following(follower_id, followee_id)) {
        return false;
    }
    
    FollowNode *follower_node = all_users[follower_id];
    FollowNode *followee_node = all_users[followee_id];
    
    // Remove from following list
    remove_from_list(follower_node->following, followee_id);
    // Remove from followers list
    remove_from_list(followee_node->followers, follower_id);
    
    // Remove from storage
    if (storage) {
        // Reconstruct follow ID: followee_id + follower_id (matches Follow::get_name)
        std::string follow_name = followee_id + follower_id;
        storage->delete_follow(follow_name);
    }
    
    return true;
}

// Get all users that a user is following
clustro::LinkedList<std::string> FollowManager::get_following(const std::string& user_id)
{
    auto it = all_users.find(user_id);
    if (it == all_users.end()) {
        return clustro::LinkedList<std::string>();
    }
    return it->second->following;
}

// Get all followers of a user
clustro::LinkedList<std::string> FollowManager::get_followers(const std::string& user_id)
{
    auto it = all_users.find(user_id);
    if (it == all_users.end()) {
        return clustro::LinkedList<std::string>();
    }
    return it->second->followers;
}

// Get suggestions for a user
clustro::LinkedList<User *> FollowManager::get_suggestions_for_user(const std::string &user_id, int max_suggestions)
{
    clustro::LinkedList<User *> suggestions;
    
    // Check if User exists
    auto User_it = all_users.find(user_id);
    if (User_it == all_users.end()) {
        return suggestions;
    }
    
    FollowNode *current_user_node = User_it->second;
    
    // Map to store suggestion scores
    std::unordered_map<std::string, int> suggestion_scores;
    
    // Get suggestions from followees of followees (2nd degree connections)
    // Using explicit iterators instead of range-based for loop
    for (auto it = current_user_node->following.begin(); it != current_user_node->following.end(); ++it) {
        std::string followee_id = *it;
        auto followee_it = all_users.find(followee_id);
        if (followee_it == all_users.end()) continue;
        
        // For each person that followee follows
        for (auto inner_it = followee_it->second->following.begin(); 
             inner_it != followee_it->second->following.end(); ++inner_it) {
            std::string suggestion_id = *inner_it;
            
            // Skip if it's the user themselves or someone they already follow
            if (suggestion_id == user_id || is_following(user_id, suggestion_id)) {
                continue;
            }
            
            // Increment score for each mutual connection
            suggestion_scores[suggestion_id]++;
        }
    }
    
    // If we don't have enough suggestions, add some random users
    // NOTE: For better performance with large user base, this should be optimized
    if (suggestion_scores.size() < max_suggestions) {
        // Use user_manager to get all users instead of iterating local nodes
        // This ensures we can suggest users we haven't encountered in graph traversal
        auto all_users_list = user_manager->get_users();
        for (auto it = all_users_list.begin(); it != all_users_list.end(); ++it) {
            std::string potential_id = it->get_id();
            
            if (potential_id == user_id || 
                is_following(user_id, potential_id) || 
                suggestion_scores.find(potential_id) != suggestion_scores.end()) {
                continue;
            }
            
            suggestion_scores[potential_id] = 1;
            if (suggestion_scores.size() >= max_suggestions * 2) break;
        }
    }
    
    if (suggestion_scores.empty()) {
        return suggestions;
    }
    
    // Convert to vector for sorting
    std::vector<std::pair<std::string, int>> scored_suggestions;
    for (const auto& pair : suggestion_scores) {
        scored_suggestions.push_back(pair);
    }
    
    // Sort by score (descending)
    std::sort(scored_suggestions.begin(), scored_suggestions.end(), 
             [](const auto &a, const auto &b) {
                 return a.second > b.second;
             });
    
    // Shuffle suggestions with same score
    std::random_device rd;
    std::mt19937 g(rd());
    
    if (!scored_suggestions.empty()) {
        int start = 0;
        for (size_t i = 1; i <= scored_suggestions.size(); ++i) {
            if (i == scored_suggestions.size() || scored_suggestions[i-1].second != scored_suggestions[start].second) {
                std::shuffle(scored_suggestions.begin() + start, scored_suggestions.begin() + i, g);
                start = i;
            }
        }
    }
    
    // Get User objects and limit to max_suggestions
    int count = 0;
    for (const auto& pair : scored_suggestions) {
        if (count >= max_suggestions) break;
        
        User* suggested_user = user_manager->search_by_id(pair.first);
        if (suggested_user != nullptr) {
            // Dereference pointer before inserting
            suggestions.insert(suggested_user);
            count++;
        }
    }
    
    return suggestions;
}

// Destructor to clean up memory
FollowManager::~FollowManager()
{
    for (auto &pair : all_users) {
        delete pair.second;
    }
    all_users.clear();
}