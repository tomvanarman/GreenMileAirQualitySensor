# US 261 Notes

## fixing grafana
We found out our login information for grafana was not there. So we had to look into that

I went onto the back-end and opened up the database folder of the grafana dashboard. I saw that the admin account still was created but it didn't have the old password because of the switching of repo's we did. So the grafana made a new admin account, with the password in the .env file. After checking that password and trying to log in, it worked. I proceeded to make a account for the client.

## fixing the CMB dashboard latest update
We found out that the text in the dashboard for latest update didn't update when we thought we updated the data.

After fixing the connection between the boxes and the back-end properly we found out that the faulty communication was the cause for not updating the update text. It now just updates!