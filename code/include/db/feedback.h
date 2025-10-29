#ifndef Feedback_H
#define Feedback_H

// =======================================
// Feedback Record Structure
// =======================================

#define FEEDBACK_NOT_REVIEWED 0
#define FEEDBACK_REVIEWED 1

typedef struct
{
    int feedbackId;
    int userId;
    char feedback[1000];
    char action[1000];
    int reviewStatus;
    // int created_timestamp;
    // int updated_timestamp;
} Feedback;

// =======================================
// Feedback method Declarations
// =======================================

// create and save feedback
void feedback_create_feedback();

// view feedback details
void feedback_view_feedback_details();

// view user feedback
void feedback_view_user_feedback();

// view all feedbacks
void feedback_view_all_feedback();

// view non review feedback
void feedback_view_non_reviewed_feedback();

// review feedback
void feedback_review_feedback();

#endif
