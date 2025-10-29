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

/*
------------------------------------------------------------------------
                        Create and Save Feedback Details
------------------------------------------------------------------------
*/

void init_feedback(Feedback *fb, int feedbackId, int userId, const char *feedbackText, const char *action, int reviewStatus)
{
    if (fb == NULL)
        return;

    fb->feedbackId = feedbackId;
    fb->userId = userId;
    fb->reviewStatus = reviewStatus;

    // Copy feedback text safely
    safe_strncpy(fb->feedback, feedbackText ? feedbackText : "", sizeof(fb->feedback));

    // Copy action safely
    safe_strncpy(fb->action, action ? action : "None", sizeof(fb->action));

    // Add timestamps
    // fb->created_timestamp = (int)time(NULL);
    // fb->updated_timestamp = fb->created_timestamp;
}

// Create and save new user
static int __fcf_prompt_user_input(int fd, const char *message, char **out)
{
    send_message(fd, message);
    if (receive_message(fd, out) < 0)
    {
        send_message(fd, "\nError receiving input.\n");
        sleep(2);
        return -2;
    }

    if (strcmp(*out, "-1") == 0)
    {
        send_message(fd, "\nOperation cancelled by user.\n");
        sleep(2);
        return -1;
    }

    return 0;
}

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

    if (__fcf_prompt_user_input(fd, "\nEnter Feedback: ", &feedback) != 0)
        goto cleanup;

    init_feedback(&tempFeedback, feedbackId, userId, feedback, "", reviewStatus);
    clear_terminal(fd);

    send_message(fd, "\nFeedback submitted successfully!\n");

    // save user to db
    if (record__save(&tempFeedback, sizeof(Feedback), FEEDBACK_DB) != 0)
        send_message(fd, "Unable to save feedback details in database.\n");
    else
        send_message(fd, "Feedback details saved to database.\n");

    send_message(fd, "\n\n\nPress enter to continue...");
    receive_message(fd, &feedback);

cleanup:
    free(feedback);
    return showStartScreen();
}

/*
------------------------------------------------------------------------
                        View Feedback Details
------------------------------------------------------------------------
*/

// Helper to print review status as text
const char *get_review_status_text(int status)
{
    return status == FEEDBACK_REVIEWED ? "Reviewed" : "Pending";
}

int __fvuf_cmp(void *rec, void *ctx)
{
    Feedback *tempFeedback = (Feedback *)rec;
    int userId = *(int *)ctx;

    return tempFeedback->userId == userId ? 1 : 0;
}

// view user feedback
void feedback_view_user_feedback()
{
    int fd = clientfd;
    int userId = logged_in_user.userId;

    void *feedbackBits = NULL;
    int count = record__search_cont(&feedbackBits, sizeof(Feedback), FEEDBACK_DB, &__fvuf_cmp, &userId);

    if (count == -1)
        goto failure;

    Feedback *Feedbacks = (Feedback *)feedbackBits;

    while (1)
    {

        clear_terminal(fd);
        send_message(fd, "\n=====================================\n");
        send_message(fd, "              My Feedbacks           \n");
        send_message(fd, "=====================================\n");
        send_message(fd, "\n\nIndex\t\t\tFeedbackId\t\t\tReviewStatus");

        int displayIndex = 1;
        for (int i = 0; i < count; i++)
        {
            int Index = displayIndex;
            int Feedback_Id = Feedbacks[i].feedbackId;
            const char *Review_Status = get_review_status_text(Feedbacks[i].reviewStatus);

            char buffer[128];
            snprintf(buffer, sizeof(buffer),
                     "\n%d\t\t\t%d\t\t\t%s",
                     i + 1, Feedback_Id, Review_Status);

            send_message(fd, buffer);
        }

        send_message(fd, "\n\nEnter the index of the feedback you want to view in detail (-1 to cancel): ");
        char *input = NULL;
        receive_message(fd, &input);
        int choice = atoi(input);
        free(input);

        if (choice == -1)
        {
            goto failure;
        }
        else if (choice < 1 || choice > count)
        {
            send_message(fd, "\nInvalid selection or cancelled.\n");
            continue;
        }

        clear_terminal(fd);
        send_message(fd, "\n=====================================\n");
        send_message(fd, makeString("              Feedback (ID:%d)          \n", Feedbacks[choice - 1].feedbackId));
        send_message(fd, "=====================================\n\n\n");

        send_message(fd, makeString("Feedback Id: %d\n\nReview Status: %s\n\nFeedback: %s\n\nAction: %s",
                                    Feedbacks[choice - 1].feedbackId,
                                    get_review_status_text(Feedbacks[choice - 1].reviewStatus),
                                    Feedbacks[choice - 1].feedback,
                                    Feedbacks[choice - 1].action));

        char *tmp = NULL;
        send_message(fd, "\n\n\nPress enter to go back...");
        receive_message(fd, &tmp);
        free(tmp);
    }

failure:
    free(feedbackBits);
    return showStartScreen();
}

/*
------------------------------------------------------------------------
                        View All Feedback Details
------------------------------------------------------------------------
*/
void feedback_view_all_feedback() {}

/*
------------------------------------------------------------------------
                        View Non-Reviewed Feedback Details
------------------------------------------------------------------------
*/
void feedback_view_non_reviewed_feedback() {}

/*
------------------------------------------------------------------------
                        Review Feedback Details
------------------------------------------------------------------------
*/
void feedback_review_feedback() {}
