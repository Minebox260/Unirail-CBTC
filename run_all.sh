#!/bin/bash
SESSION="Unirail"

echo "Starting tmux session..."

tmux new-session -d -s $SESSION -n $SESSION

tmux send-keys -t $SESSION:0 "make run-supervisor" C-m
tmux select-pane -t $SESSION:0
tmux select-pane -T "Supervisor"

sleep 5

tmux split-window -v -p 50 -t $SESSION:0
tmux send-keys -t $SESSION:0.1 "make run-onboard-1" C-m
tmux select-pane -t $SESSION:0.1
tmux select-pane -T "Onboard1"

tmux split-window -h -t $SESSION:0.1 -p 67
tmux send-keys -t $SESSION:0.2 "make run-onboard-2" C-m
tmux select-pane -t $SESSION:0.2
tmux select-pane -T "Onboard2"

tmux split-window -h -t $SESSION:0.2 -p 50
tmux send-keys -t $SESSION:0.3 "make run-onboard-3" C-m
tmux select-pane -t $SESSION:0.3
tmux select-pane -T "Onboard3"

gnome-terminal --wait --title $SESSION -- bash -c "tmux attach -t $SESSION"
echo "Cleaning up..."
tmux kill-session -t $SESSION
