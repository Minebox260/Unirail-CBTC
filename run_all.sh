#!/bin/bash
SESSION="Unirail"

echo "Starting tmux session..."

tmux new-session -d -s $SESSION -n $SESSION

tmux send-keys -t $SESSION:0 "make run-rbc" C-m
tmux select-pane -t $SESSION:0
tmux select-pane -T "RBC"

sleep 5

tmux split-window -v -p 50 -t $SESSION:0
tmux send-keys -t $SESSION:0.1 "make run-evc-1" C-m
tmux select-pane -t $SESSION:0.1
tmux select-pane -T "EVC1"

tmux split-window -h -t $SESSION:0.1 -p 67
tmux send-keys -t $SESSION:0.2 "make run-evc-2" C-m
tmux select-pane -t $SESSION:0.2
tmux select-pane -T "EVC2"

tmux split-window -h -t $SESSION:0.2 -p 50
tmux send-keys -t $SESSION:0.3 "make run-evc-3" C-m
tmux select-pane -t $SESSION:0.3
tmux select-pane -T "EVC3"

gnome-terminal --wait --title $SESSION -- bash -c "tmux attach -t $SESSION"
echo "Cleaning up..."
tmux kill-session -t $SESSION
