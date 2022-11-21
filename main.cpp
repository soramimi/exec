
#include <cstdio>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <vector>
#include <string>
#include <cstring>

std::string hoge()
{
	const int R = 0;
	const int W = 1;

	int child_to_parent[2], parent_to_child[2];

	if (pipe(child_to_parent) < 0) {
		return {};
	}

	if (pipe(parent_to_child) < 0) {
		close(child_to_parent[R]);
		close(child_to_parent[W]);
		return {};
	}

	pid_t pid = fork();
	if (pid < 0) {
		close(child_to_parent[R]);
		close(child_to_parent[W]);
		close(parent_to_child[R]);
		close(parent_to_child[W]);
		return {};
	}

	if (pid == 0) {
		const int argc = 2;
		char **argv = (char **)alloca(sizeof(char *) * (argc + 1));
		argv[0] = strdup("uname");
		argv[1] = strdup("-a");
		argv[argc] = nullptr;

		close(parent_to_child[W]);
		close(child_to_parent[R]);
		dup2(parent_to_child[R], 0);
		dup2(child_to_parent[W], 1);
		close(parent_to_child[R]);
		close(child_to_parent[W]);

		execvp(argv[0], argv);
		exit(0);
	}

	close(parent_to_child[R]);
	close(child_to_parent[W]);
	int fd_w = parent_to_child[W];
	int fd_r = child_to_parent[R];

	std::vector<char> text;

	close(fd_w);

	int wstatus = 0;
	if (waitpid(pid, &wstatus, 0) == pid) {
		if (WIFEXITED(wstatus)) {
			fd_set fds;
			FD_ZERO(&fds);
			FD_SET(fd_r, &fds);
			int ret = WEXITSTATUS(wstatus);
			(void)ret;
			for (int i = 0; i < 10; i++) {
				struct timeval tv;
				tv.tv_sec = 0;
				tv.tv_usec = 1000;
				int n = select(fd_r + 1, &fds, nullptr, nullptr, &tv);
				if (n > 0 && FD_ISSET(fd_r, &fds)) {
					char tmp[1024];
					n = read(fd_r, tmp, sizeof(tmp));
					if (n < 1) break;
					text.insert(text.end(), tmp, tmp + n);
				}
			}
		}
	}

	close(fd_r);

	if (text.empty()) return {};

	return std::string(text.data(), text.size());
}

int main()
{
	std::string s = hoge();
	puts(s.c_str());
	return 0;
}
