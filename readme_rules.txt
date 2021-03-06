Rules for usage of the git repository:

* NEVER commit large files!
	* Think hard before commiting anything over 1k
	* Discuss with others before commiting anything over 10k
	* Punishment will be having to fix the git commit history
       on your machine and any machine your changes get propagated to!
	* EVERY git repository contains the full development history,
		so even if the file is deleted in a future commit,
		it still weighs down everyone's repository FOREVER
		unless you properly fix the history, which can be painful
		for new users.

* ALWAYS use a good <72 character, one line summary as the top
	line of your commit message.
	* Add details below, after a blank line wrapping to 72 characters.
	* Focus on adding semantic information that is ~not
		already part of the diff/changelist
	* Doing this makes it possible to browse and actually make
		good use of the history!

* ALL auto-generated files should be added to a .gitignore file
	so that they are NOT versioned by git
	* echo "filename" >> ./.gitignore
	* git add ./.gitignore

* Try hard not to push broken code to the master branch on esat or the carousel
	* If you know what you're doing, you can create additional branches
	* on the carousel and/or esat that can contain broken code.

* Strive to add build rules for all autogenerated files
	* CMake can be a bitch. Ask for help 

* Try to group related changes together into commits
	* Err on the side of smaller commits:
	* If you need to ammend your local, unpushed history,
		it is easier to combine(squash) commits than to split them.
