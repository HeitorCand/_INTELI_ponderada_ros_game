#!/bin/zsh
micromamba activate ros_env

export DYLD_FALLBACK_LIBRARY_PATH="/opt/micromamba/envs/ros_env/lib:$DYLD_FALLBACK_LIBRARY_PATH"
export CMAKE_PREFIX_PATH=/opt/micromamba/envs/ros_env:$CMAKE_PREFIX_PATH
export COLCON_DEFAULTS_FILE="/Users/heitor.candido/Documents/Ponderadas/culling_games/defaults.yml"

colcon build
source install/setup.zsh