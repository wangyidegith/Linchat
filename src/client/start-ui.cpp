#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main (int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: ./elfname <username>\n");
        return -1;
    }
    const char* username = argv[1];
    const char* envp[] = {"PS1=\"\"", NULL};
    system("tmux new-session -d -s linchat");
    execle("./input2all", "input2all", username, NULL, envp);
    system("tmux split-window -h -p 50");
    execle("./output2all", "output2all", NULL, envp);
    system("tmux split-window -h -p 5");
    execle("./output2users", "output2users", NULL, envp);
    system("tmux select-pane -L");
    system("tmux split-window -v -p 60");
    execle("./output2single", "output2single", NULL, envp);
    system("tmux select-pane -L");
    system("tmux split-window -v");
    execle("./input2single", "input2single", username, NULL, envp);
    system("tmux split-window -v -p 95");
    execle("./input2dstname", "input2dstname", NULL, envp);
    system("tmux attach-session -t linchat");
    return 0;
}
