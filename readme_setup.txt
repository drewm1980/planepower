# Steps for getting the carousel code running for a new developer:

Create new user on planepower-pc (or wherever)
>> sudo adduser bob

# Get the repository checked out:
1. >> ssh-keygen (hit enter repeatedly to select defaults)
2. Send the ~/.ssh/id_rsa.pub to the esat helpdesk attached to a ticket requesting access to the "planepower" git repository
3. >> sudo apt-get install git-core gitk
4. >> cd ~
5. >> git clone git@git.esat.kuleuven.be:planepower.git

6. Copy the template .gitconfig and correct your name 
	and e-mail address in ~/.gitconfig
>> cp planepower/.gitconfig_sample ~/.gitconfig
>> gedit ~/.gitconfig

# Read the rules, love the rules, live by the rules:
7. read ~/planepower/readme_rules.txt





