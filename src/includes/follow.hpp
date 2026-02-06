#pragma once
#ifndef FOLLOW_HPP
#define FOLLOW_HPP

// 
// Written by: khaixu30
// 


#include <iostream>
#include "utils/LinkedList.hpp"
#include "user.hpp"
#include "utils/utils.hpp"
#include <sstream> // Required for parsing

class Follow {
private:
    std::string followee_id;   // The user being followed
    std::string follower_id;   // The user who is following
    std::string timestamps;

    // Helper function to parse data from a string (used by constructor)
    void parse(const std::string& data) {
        std::istringstream stream(data);
        std::string line;
        while (std::getline(stream, line)) {
            auto pos = line.find(":");
            if (pos == std::string::npos) continue;
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);
            
            if (key == "followee") {
                followee_id = value;
            } else if (key == "follower") {
                follower_id = value;
            } else if (key == "timestamps") {
                timestamps = value;
            }
        }
    }

public:
    // Default constructor
    Follow() = default;

    // Constructor for creating a new follow relationship
    Follow(User& followee, User& follower) {
        this->followee_id = followee.get_id();
        this->follower_id = follower.get_id();
        this->timestamps = clustro::get_current_time();
    }

    // Constructor for loading a follow relationship from storage
    Follow(const std::string& follow_data) {
        parse(follow_data);
    }

    // Serializes the follow object's data into a string
    std::string stringify() const {
        std::ostringstream oss;
        oss << "followee:" << followee_id << "\nfollower:" << follower_id << "\ntimestamps:" << timestamps;
        return oss.str();
    }

    // Creates a unique key for this follow relationship.
    // This is used for the filename by the Storage class.
    std::string get_id() const {
        // Use a separator to ensure uniqueness (e.g., "user1->user2")
        return followee_id + "->" + follower_id;
    }

    std::string get_name(){
        return followee_id + follower_id;
    }

    // --- Getters ---
    std::string get_followee_id() const {
        return followee_id;
    }

    std::string get_follower_id() const {
        return follower_id;
    }

    std::string get_timestamps() const {
        return timestamps;
    }
};

#endif