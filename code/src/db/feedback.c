// feedback.c

//============================================================================

// This file contains logic and UI related to feedback

//============================================================================

#include "db/feedback.h"
#include "db/user.h"
#include "config.h"
#include "helper.h"
#include "db/user.h"
#include "file_operation.h"
#include <time.h>
#include <stdio.h>
#include "start_screen.h"
#include "communication.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

static void __init_feedback(Feedback *fb, int feedbackId, int userId, const char *feedbackText, const char *action, int reviewStatus)
{
    if (fb == NULL)
        return;

    fb->feedbackId = feedbackId;
    fb->userId = userId;
    fb->reviewStatus = reviewStatus;
    safe_strncpy(fb->feedback, feedbackText ? feedbackText : "", sizeof(fb->feedback));
    safe_strncpy(fb->action, action ? action : "None", sizeof(fb->action));

    // Add timestamps
    // fb->created_timestamp = (int)time(NULL);
    // fb->updated_timestamp = fb->created_timestamp;
}

static const char *__get_review_status_text(int status)
{
    return status == FEEDBACK_REVIEWED ? "Reviewed" : "Pending";
}

static void __display_feedbacks(int fd, int count, Feedback *Feedbacks, char *header)
{
    clear_terminal(fd);
    send_message(fd, "\n=====================================\n");
    send_message(fd, makeString("              %s           \n", header));
    send_message(fd, "=====================================\n");
    send_message(fd, "\n\nIndex\t\t\tFeedbackId\t\tReviewStatus");

    for (int i = 0; i < count; i++)
    {
        int Feedback_Id = Feedbacks[i].feedbackId;
        const char *Review_Status = __get_review_status_text(Feedbacks[i].reviewStatus);
        send_message(fd, makeString("\n%d\t\t\t%d\t\t\t%s",
                                    i + 1, Feedback_Id, Review_Status));
    }
}

static void __display_a_feedback(int fd, int choice, Feedback *Feedbacks)
{
    clear_terminal(fd);
    send_message(fd, "\n=====================================\n");
    send_message(fd, makeString("              Feedback (ID:%d)          \n", Feedbacks[choice - 1].feedbackId));
    send_message(fd, "=====================================\n\n\n");

    send_message(fd, makeString("Feedback Id: %d\n\nReview Status: %s\n\nFeedback: %s\n\nAction: %s",
                                Feedbacks[choice - 1].feedbackId,
                                __get_review_status_text(Feedbacks[choice - 1].reviewStatus),
                                Feedbacks[choice - 1].feedback,
                                Feedbacks[choice - 1].action));
}

//============================================================================

// Create new feedback

//============================================================================

void feedback_create_feedback()
{
    int fd = clientfd;
    Feedback tempFeedback;

    int userId = logged_in_user.userId;
    char *feedback = NULL;

    int reviewStatus = FEEDBACK_NOT_REVIEWED;
    int feedbackId = generateUniqueFeedbackId();

    clear_terminal(fd);
    send_message(fd, "\n=====================================\n");
    send_message(fd, "     Feedback Creation Portal        \n");
    send_message(fd, "=====================================\n");

    send_message(fd, "\n Use -1 to cancel \n\n");

    if (prompt_user_input(fd, "\nEnter Feedback: ", &feedback) != 0)
        goto cleanup;

    __init_feedback(&tempFeedback, feedbackId, userId, feedback, "", reviewStatus);

    clear_terminal(fd);
    send_message(fd, "\nFeedback submitted successfully!\n");

    // save user to db
    if (record__save(&tempFeedback, sizeof(Feedback), FEEDBACK_DB) != 0)
        send_message(fd, "\nUnable to save feedback details in database.\n");
    else
        send_message(fd, "\nFeedback details saved to database.\n");

    waitTillEnter(fd);

cleanup:
    free(feedback);
    return showStartScreen();
}

//============================================================================

// View Feedback Details

//============================================================================

static int __find_feedback_based_on_userId(void *rec, void *ctx)
{
    Feedback *tempFeedback = (Feedback *)rec;
    int userId = *(int *)ctx;

    return tempFeedback->userId == userId ? 1 : 0;
}

// View Feedback Details
void feedback_view_user_feedback()
{
    int fd = clientfd;
    int userId = logged_in_user.userId;

    void *feedbackBits = NULL;
    int count = record__search_cont(&feedbackBits, sizeof(Feedback), FEEDBACK_DB, &__find_feedback_based_on_userId, &userId);

    if (count == -1)
        goto cleanup;

    Feedback *Feedbacks = (Feedback *)feedbackBits;

    while (1)
    {
        __display_feedbacks(fd, count, Feedbacks, "My Feedbacks");

        char *input = NULL;
        prompt_user_input(fd, "\n\nEnter the index of the feedback you want to view in detail (-1 to cancel): ", &input);

        int choice = atoi(input);
        free(input);

        if (choice == -1)
        {
            goto cleanup;
        }
        else if (choice < 1 || choice > count)
        {
            send_message(fd, "\nInvalid selection or cancelled.\n");
            continue;
        }

        __display_a_feedback(fd, choice, Feedbacks);
        waitTillEnter(fd);
    }

cleanup:
    free(feedbackBits);
    return showStartScreen();
}

//============================================================================

// Review Feedback Details

//============================================================================

static int __find_feedback_based_on_feedbackId(void *rec, void *ctx)
{
    Feedback *tempFeedback = (Feedback *)rec;
    int feedbackID = *(int *)ctx;

    return tempFeedback->feedbackId == feedbackID ? 1 : 0;
}

static void __feedback_create_action(int feedback_Id, int userId, char *feedback)
{
    int fd = clientfd;
    Feedback updatedFeedback;
    char *action = NULL;

    int reviewStatus = FEEDBACK_REVIEWED;
    int feedbackId = feedback_Id;

    clear_terminal(fd);
    send_message(fd, "\n======================================\n");
    send_message(fd, "         Action Creation Portal         \n");
    send_message(fd, "======================================\n");

    send_message(fd, "\nUse -1 to cancel \n\n");

    if (prompt_user_input(fd, "\nEnter Action: ", &action) != 0)
        goto cleanup;

    __init_feedback(&updatedFeedback, feedbackId, userId, feedback, action, reviewStatus);
    clear_terminal(fd);

    send_message(fd, "\nAction submitted successfully!\n");

    // save user to db
    Feedback tempFeedback;
    int pos = record__search(&tempFeedback, sizeof(Feedback), FEEDBACK_DB, &__find_feedback_based_on_feedbackId, &feedback_Id);

    if (record__update(&updatedFeedback, sizeof(Feedback), FEEDBACK_DB, pos))
        send_message(fd, "\nUnable to save action details in database.\n");
    else
        send_message(fd, "\nAction details saved to database.\n");

cleanup:
    free(action);
    return;
}

static int __find_non_reviewed_feedbacks(void *rec, void *ctx)
{
    Feedback *tempFeedback = (Feedback *)rec;
    int not_reviewed = *(int *)ctx;

    return tempFeedback->reviewStatus == not_reviewed ? 1 : 0;
}

// Review Feedback Details
void feedback_reviewCreate_action()
{
    int fd = clientfd;
    int not_reviewed = FEEDBACK_NOT_REVIEWED;
    void *feedbackBits = NULL;
    char *temp = NULL;

    while (1)
    {
        int count = record__search_cont(&feedbackBits, sizeof(Feedback), FEEDBACK_DB, &__find_non_reviewed_feedbacks, &not_reviewed);
        if (count == -1)
            goto failure;

        Feedback *Feedbacks = (Feedback *)feedbackBits;

        // Displaying all non-reviewed feedbacks
        __display_feedbacks(fd, count, Feedbacks, "Non-Reviewed Feedbacks");

        // Viewing Non-Reviewed Feedbacks in detail

        prompt_user_input(fd, "\n\nEnter the index of the non-reviewed feedback you want to view in detail (-1 to cancel): ", &temp);
        int choice = atoi(temp);

        if (choice == -1)
        {
            goto failure;
        }
        else if (choice < 1 || choice > count)
        {
            send_message(fd, "\nInvalid selection or cancelled.\n");
            sleep(2);
            continue;
        }

        __display_a_feedback(fd, choice, Feedbacks);

        // Review non-reviewed feedback based on index value
        prompt_user_input(fd, makeString("\n\nReview non-reviewed feedback (ID:%d) (1 to review, 0 to not review): ", Feedbacks[choice - 1].feedbackId), &temp);
        int reviewChoice = atoi(temp);

        if (reviewChoice == 1)
            __feedback_create_action(Feedbacks[choice - 1].feedbackId, Feedbacks[choice - 1].userId, Feedbacks[choice - 1].feedback);
        else if (reviewChoice == 0)
            continue;
        else
            send_message(fd, "\nInvalid choice.\n");

        waitTillEnter(fd);
    }

failure:
    free(feedbackBits);
    free(temp);
    return showStartScreen();
}

// /*
// ------------------------------------------------------------------------
//                         View All Feedback Details
// ------------------------------------------------------------------------
// */

// void feedback_view_all_feedback() {}

// /*
// ------------------------------------------------------------------------
//                         View Non-Reviewed Feedback Details
// ------------------------------------------------------------------------
// */

// void feedback_view_non_reviewed_feedback() {}