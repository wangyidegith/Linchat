#!/bin/bash

# Check if the username argument is provided
if [ $# -ne 1 ]; then
    echo "Usage: $0 <username>"
    exit 1
fi

username="$1"

# Create a new tmux session
tmux new-session -d -s linchat

# (1) input2all
tmux send-keys -t linchat:0 "./input2all $username" C-m
# tmux send-keys -t linchat:0 "export PS1=''" C-m
# tmux send-keys -t linchat:0 C-l

# (2) output2all
tmux split-window -h -p 50
tmux send-keys -t linchat:0 "./output2all" C-m
# tmux send-keys -t linchat:0 "export PS1=''" C-m
# tmux send-keys -t linchat:0 C-l

# (3) output2users
tmux split-window -h -p 5
tmux send-keys -t linchat:0 "./output2users" C-m
# tmux send-keys -t linchat:0 "export PS1=''" C-m
# tmux send-keys -t linchat:0 C-l

# (4) output2single
tmux select-pane -L
tmux split-window -v -p 60
tmux send-keys -t linchat:0 "./output2single" C-m
# tmux send-keys -t linchat:0 "export PS1=''" C-m
# tmux send-keys -t linchat:0 C-l

# (5) input2single
tmux select-pane -L
tmux split-window -v
tmux send-keys -t linchat:0 "./input2single $username" C-m
# tmux send-keys -t linchat:0 "export PS1=''" C-m
# tmux send-keys -t linchat:0 C-l

# (6) input2dstname
tmux split-window -v -p 5
tmux send-keys -t linchat:0 "./input2dstname" C-m
# tmux send-keys -t linchat:0 "export PS1=''" C-m
# tmux send-keys -t linchat:0 C-l

# (7) attach to the session
tmux attach-session -t linchat

