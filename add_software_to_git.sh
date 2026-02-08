#! /bin/bash

# Add the software directory to the git repository
# This is a workaround to avoid adding the nrf_connect_project directory as a submodule
# nRF Connect creates a .git directory during project initialization at software/nrf_connect_prj/.git, 
# which prevents adding the nrf_connect_prj directory directly to the repository.
# Workaround is to rename the .git dire

script_dir=$(dirname "$0")
mv $script_dir/software/rear_light_peripheral/.git $script_dir/software/.git_backup_rear_light_peripheral
git add $script_dir/software/rear_light_peripheral
mv $script_dir/software/.git_backup_rear_light_peripheral $script_dir/software/rear_light_peripheral/.git

mv $script_dir/software/front_light_central/.git $script_dir/software/.git_backup_front_light_central
git add $script_dir/software/front_light_central
mv $script_dir/software/.git_backup_front_light_central $script_dir/software/front_light_central/.git

exit 0
