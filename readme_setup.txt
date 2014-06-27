# Steps for getting the flight software running for a new developer:

# Get the planepower git repository checked out:
1. >> ssh-keygen (hit enter repeatedly to select defaults)
2. Send the ~/.ssh/id_rsa.pub to the esat helpdesk attached to a ticket requesting access to the "planepower" git repository
3. >> sudo apt-get install git-core gitk
4. >> cd ~
5. >> git clone git@git.esat.kuleuven.be:planepower.git

6. Copy the template .gitconfig and correct your name 
	and e-mail address in ~/.gitconfig
>> cp planepower/.gitconfig_sample ~/.gitconfig
>> gedit ~/.gitconfig

7. Run setup_precommit_hook

# Read the rules, love the rules, live by the rules:
8. read ~/planepower/readme_rules.txt

9. Go follow the directions for building in readme_building.txt



   
