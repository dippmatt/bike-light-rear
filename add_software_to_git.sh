#! /bin/bash

# Add the software directory to the git repository
# This is a workaround to avoid adding the nrf_connect_project directory as a submodule
# nRF Connect creates a .git directory during project initialization at software/nrf_connect_prj/.git, 
# which prevents adding the nrf_connect_prj directory directly to the repository.
# Workaround is to rename the .git dire

script_dir=$(dirname "$0")
mv $script_dir/software/nrf_connect_prj/.git $script_dir/software/nrf_connect_prj/.git_backup
git add $script_dir/software/nrf_connect_prj
mv $script_dir/software/nrf_connect_prj/.git_backup $script_dir/software/nrf_connect_prj/.git

exit 0