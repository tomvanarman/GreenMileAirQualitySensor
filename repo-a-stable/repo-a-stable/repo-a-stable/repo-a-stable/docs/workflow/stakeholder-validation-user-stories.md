# Stakeholder Validation with Ruth (Green Mile)

## Introduction

During the Green Mile project, we created user stories based on input from our client, who we initially considered to be the main stakeholder. However, the client has a more technical perspective and is not the actual end user of the product. After receiving feedback from our teacher, we realized that our user stories were not yet validated from the perspective of the real end user, Ruth.

This means that our current user stories may not fully reflect the actual needs and goals of the person the product is ultimately intended for. Therefore, validation with the end user is necessary to improve the quality and relevance of our requirements.

## Table of Contents

- [Introduction](#introduction)
- [Table of Contents](#table-of-contents)
- [1. Current User Stories](#1-current-user-stories)
  - [1.1 Epic](#11-epic)
  - [1.2 Sub-epics](#12-sub-epics)
  - [1.3 User Stories](#13-user-stories)
- [2. Identified Issues](#2-identified-issues)
- [3. Validation Plan](#3-validation-plan)
- [4. Expected Outcome](#4-expected-outcome)
- [5. Questions for Stakeholder](#5-questions-for-stakeholder)
- [6. Stakeholder Feedback](#6-stakeholder-feedback)
  - [6.1 Key Insights](#61-key-insights)
  - [6.2 Answers per Question](#62-answers-per-question)
- [7. Improvements to User Stories](#7-improvements-to-user-stories)
- [8. Reflection](#8-reflection)

## 1. Current User Stories

Currently, we have defined the following user stories:

### 1.1 Epic

- As a resident, I want real-time data to assess air quality, so that I know that the quality of air meets national standards in the area the device runs.

### 1.2 Sub-epics

- As a resident, I want a proof-of-concept device to test real-time air-quality monitoring, so I can validate its usefulness before full deployment.
- As a resident, I want my air quality readings to be saved in the cloud, so I can always have access to the readings.
- As a resident, I want a dashboard showing real-time air-quality levels so I can immediately see whether local air quality meets national standards.

### 1.3 User Stories

- As a resident, I want my air quality to be saved on cloud storage so I can access it later anywhere.
- As a resident, I want to be able to get my data no matter where I am, so I can view if my air quality meets national standards.
- As a resident, I want to have clear documentation of how to build and then use the air quality sensor box.
- As a resident, I want a nice and compact design that is also functional, so that the product is not larger than necessary.
- As a resident, I want there to be a connection to a cloud storage, so my air quality can get saved for me to see at a later point.
- As a resident, I want to be able to measure particulate matter with the device, so that I can use the measured data to compare with national standards.
- As a resident, I want to be able to measure humidity and temperature with the device, so that I can use the measured data to compare with national standards.
- As a user, I want the device to work independently without being connected to a computer, so that I can use it on the marine terrain.

## 2. Identified Issues

During analysis of the current user stories, we identified overlapping requirements, particularly regarding cloud storage and data accessibility. These overlaps indicate redundancy and a lack of clarity in how requirements are defined.

The following user stories describe similar functionality:

- As a resident, I want my air quality readings to be saved in the cloud, so I can always have access to the readings.
- As a resident, I want my air quality to be saved on cloud storage so I can access it later anywhere.
- As a resident, I want there to be a connection to a cloud storage, so my air quality can get saved for me to see at a later point.

These user stories all focus on storing and accessing data, and can likely be merged or refined into a single, clearer requirement.

## 3. Validation Plan

To improve the quality of our user stories, we will validate them with Ruth as the end user.

We will:
- Contact Ruth and schedule a meeting
- Ask structured questions about her vision, goals, and expectations
- Verify whether the current user stories align with her needs
- Identify missing or unnecessary requirements
- Document her feedback in a structured format

Based on this validation:
- We will refine or merge overlapping user stories
- Rewrite unclear or ambiguous requirements
- Ensure all user stories reflect real user needs

## 4. Expected Outcome

By validating our user stories with the end user, we expect to:
- Reduce duplication in requirements
- Improve clarity and relevance of user stories
- Ensure alignment between development and user needs
- Strengthen the overall development process through better requirement management

## 5. Questions for Stakeholder

During the stakeholder conversation, we will use our predefined questions as a base. Based on the stakeholder’s answers, we will ask follow-up questions to gain more specific insights into the requirements of the system. These follow-up questions help us better understand what needs to be developed and improve the quality of our user stories.

### 1. Vision

**Main question:**  
- What is your vision for the Green Mile project?

**Follow-up questions:**  
- What does that mean for what we specifically need to build?  
- What should our final product definitely include?  
- What would make the project successful for you?  

---

### 2. Target Audience

**Main question:**  
- Who is the Climate Box intended for?

**Follow-up questions:**  
- Would that person actually use the device themselves?  
- How often would they use it?  
- Do they have technical knowledge or not?  

---

### 3. Purpose of Measuring Air Quality

**Main question:**  
- What is the main purpose of measuring air quality in Amsterdam?

**Follow-up questions:**  
- What should users do with that information?  
- What decisions should they be able to make?  
- Which data is most important for that purpose?  

---

### 4. Location (Stadhouderskade)

**Main question:**  
- Why was the Stadhouderskade chosen as the location?

**Follow-up questions:**  
- Does this affect how we should design the device?  
- Are there environmental conditions we should consider?  
- Should the device work in similar locations as well?  

---

### 5. Build-it-yourself Aspect

**Main question:**  
- Why is it important that people can build their own Climate Box?

**Follow-up questions:**  
- How easy should it be to build?  
- Should non-technical users be able to do it?  
- What kind of instructions would be needed?  

---

### 6. Understanding Air Quality

**Main question:**  
- How should users be able to understand whether the air quality is good or bad?

**Follow-up questions:**  
- Do you prefer simple indicators (colors) or detailed data?  
- Should users receive alerts or warnings?  
- Should it directly match official standards?  

---

### 7. Expectations for This Project

**Main question:**  
- What do you expect from us within this project?

**Follow-up questions:**  
- What are the most important features for you?  
- What should we prioritize first?  
- What would define a successful result for you?  

## 6. Stakeholder Feedback

### 6.1 Key Insights

- **Vision:**  
  The project should create a working prototype that increases awareness of air quality. The goal is to help people make informed decisions (e.g. choosing cleaner routes).

- **Target audience:**  
  Residents and people in urban areas (e.g. Amsterdam) who are affected by air pollution and want to understand air quality in their environment.

- **Purpose of the Climate Box:**  
  To measure air quality and provide insights that help users make decisions based on pollution levels.

- **Important features:**  
  - Clear and simple visualization of air quality (e.g. colors or other intuitive indicators to show good or bad air quality)  
  - Notifications or visual alerts to draw attention when air quality is poor  
  - Time-tracked data (including seasonal trends)  
  - A working prototype that demonstrates functionality  

- **Feedback on current ideas:**  
  - The device should focus more on usability and awareness  
  - Data should not only be real-time, but also tracked over time  
  - The design should clearly communicate air quality to users in an intuitive way  

---

### 6.2 Answers per Question

| Question | Answer | Impact on User Stories |
|----------|--------|----------------------|
| What is your vision for the Green Mile project? | A working prototype that increases awareness of air quality and helps people make decisions (e.g. choosing routes). | User stories should focus on awareness and decision-making, not just data collection. |
| Who is the Climate Box intended for? | Residents and people affected by air pollution in urban areas. | User stories should reflect non-technical users and ease of use. |
| What is the main purpose of measuring air quality in Amsterdam? | To help people make decisions based on air quality (e.g. choosing cleaner routes). | Add user stories related to actionable insights, not just displaying data. |
| Why was the Stadhouderskade chosen as the location? | It is one of the most polluted streets and many related companies are located there. | Device should be suitable for highly polluted environments and possibly similar locations. |
| Why is it important that people can build their own Climate Box? | To increase accessibility and awareness, and allow more people to use it. | Keep user story about documentation, but ensure it is simple and user-friendly. |
| How should users be able to understand whether the air quality is good or bad? | The information should be displayed in a clear and intuitive way (e.g. using colors or signals), possibly supported by alerts to draw attention. | User stories should focus on clear and intuitive visualization, without limiting the solution to one specific implementation (e.g. LEDs). |

## 7. Improvements to User Stories

Based on the stakeholder feedback, several improvements can be made to how the user stories are written:

- **Make user stories more specific and actionable:**  
  Some user stories are currently too broad. They should clearly describe what the user needs and how the system should respond.

- **Focus on the end user (Ruth) instead of assumptions:**  
  The original user stories were partly based on technical assumptions. They should be rewritten to reflect real user needs, such as awareness and decision-making.

- **Reduce duplication:**  
  Multiple user stories describe similar functionality (e.g. cloud storage and data access). These should be merged into a single, clear user story.

- **Include the purpose of the feature more clearly:**  
  The “so that” part should better reflect why the feature is important, for example helping users make decisions based on air quality.

- **Emphasize usability and simplicity:**  
  User stories should reflect that the system must be easy to understand for non-technical users.

- **Add user interaction elements:**  
  Based on feedback, user stories should include how users interact with the system (e.g. visual indicators like LED colors or alerts).

- **Incorporate time-based insights:**  
  User stories should include the need for tracking and understanding air quality over time (e.g. trends or seasonal data).

- **Align user stories with real-world use cases:**  
  The stories should reflect practical situations, such as choosing cleaner routes based on air quality data.

- **Avoid technical wording where possible:**  
  User stories should be written from the user’s perspective, not from a technical implementation perspective.

## 8. Reflection

During the stakeholder validation, we gained more specific insights into what the end user actually needs. Initially, our questions were too broad and focused on the overall project rather than our specific contribution.

By asking follow-up questions, we were able to better understand what is expected from our system. The stakeholder emphasized the importance of awareness, usability, and clear communication of air quality data.

This helped us improve our user stories by:
- Focusing more on user interaction and decision-making  
- Reducing unnecessary or overlapping requirements  
- Adding features such as visual indicators and time-based data  

Overall, this validation improved the alignment between our development work and the actual needs of the end user.