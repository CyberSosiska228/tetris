#include <bits/extc++.h>
#include <fcntl.h>

#define DEFAULT "\033[40m"
#define CYAN "\033[106m"
#define BLUE "\033[44m"
#define ORANGE "\033[43m"
#define YELLOW "\033[103m"
#define GREEN "\033[42m"
#define PURPLE "\033[45m"
#define RED "\033[41m"


std::mt19937 gen(std::chrono::high_resolution_clock::now().time_since_epoch().count());

void read_input(const int buf_input_sz, int *buf_input) {
    const int reload_time = 5;
    const int max_input = 500;
    const int rd_sz = 1024;
    char buf[rd_sz];
    int fd = open("/dev/input/event11", O_RDONLY);
    assert(fd != -1);

    int *disable = (int *) std::malloc(sizeof(int) * max_input);

    buf_input[0] = 1;
    int last_ev = 0;
    while (true) {
        read(fd, buf, rd_sz * sizeof(char));
        int ev = buf[42];
        if (ev == 0) {
            ev = last_ev;
        }
        last_ev = ev;

        if (disable[ev]) {
            disable[ev] = false;
            continue;
        }
        disable[ev] = true;

        buf_input[buf_input[0]] = ev;
        buf_input[0]++;

        if (buf_input[0] == buf_input_sz) {
            buf_input[0] = 1;
        }
    }
}

struct point {
    int x, y;

    point() {}
    point(int _x, int _y) {
        x = _x;
        y = _y;
    }
};

struct figure {
    int rt_num;
    int sq_sz;
    int clr;
    int rot;
    point ps;

    int ***scr;

    void read(int w) {
        rot = 0;
        std::cin >> rt_num >> sq_sz >> clr;
        ps = point(0, w / 2 - sq_sz / 2);
        scr = (int ***) std::malloc(sizeof(int **) * rt_num);
        for (int i = 0; i < rt_num; i++) {
            scr[i] = (int **) std::malloc(sizeof(int *) * sq_sz);
            for (int j = 0; j < sq_sz; j++) {
                scr[i][j] = (int *) std::malloc(sizeof(int) * sq_sz);
                for (int k = 0; k < sq_sz; k++) {
                    char t;
                    std::cin >> t;
                    scr[i][j][k] = (t == '#' ? clr : 0);
                }
            }
        }
    }

    figure() {}
};

struct screen {
    int w, h;
    int str_w, str_h;
    int **scr;

    std::string get_color(int x) {
        switch (x) {
            case 0:
                return DEFAULT;
            case 1:
                return CYAN;
            case 2:
                return BLUE;
            case 3:
                return ORANGE;
            case 4:
                return YELLOW;
            case 5:
                return GREEN;
            case 6:
                return PURPLE;
            case 7:
                return RED;
            case 8:
                return DEFAULT;
        }
        return "-1";
    }

    void draw_next_fg(figure nxt, int l, int it) {
        if (l == 0 && it == str_h / 2) {
            for (int i = 0; i < str_w; i++) {
                std::cout << " ";
            }
            std::cout << "Next:";
        } else if (l >= 1 && l <= nxt.sq_sz) {
            for (int i = 0; i < str_w; i++) {
                std::cout << " ";
            }
            for (int i = 0; i < nxt.sq_sz; i++) {
                std::cout << get_color(nxt.scr[0][l - 1][i]);
                for (int j = 0; j < str_w; j++) {
                    std::cout << " ";
                }
                std::cout << DEFAULT;
            }
        }
        std::cout << '\n';
    }

    void draw(figure fg, int score, int level, figure nxt) {
        for (int i = 1; i < h - 1; i++) {
            for (int f = 0; f < str_h; f++) {
                for (int j = 1; j < w - 1; j++) {
                    std::cout << get_color(scr[i][j]);

                    if (fg.ps.x <= i && fg.ps.x + fg.sq_sz > i && fg.ps.y <= j && fg.ps.y + fg.sq_sz > j && fg.scr[fg.rot][i - fg.ps.x][j - fg.ps.y] != 0) {
                        std::cout << get_color(fg.scr[fg.rot][i - fg.ps.x][j - fg.ps.y]);
                    }

                    for (int k = 0; k < str_w; k++) {
                        std::cout << ' ';
                    }
                    std::cout << DEFAULT;
                }
                std::cout << '|';
                draw_next_fg(nxt, i - 1, f);
            }
        }
        for (int i = 1; i < w - 1; i++) {
            for (int j = 0; j < str_w; j++) {
                std::cout << '-';
            }
        }
        std::cout << "\n\n";
        std::cout << "Score: " << score << '\n';
        std::cout << "Lvel:  " << level << '\n';
    }

    void build(int _w, int _h) {
        w = _w;
        h = _h;
        scr = (int **) std::malloc(sizeof(int *) * h);
        for (int i = 0; i < h; i++) {
            scr[i] = (int *) std::malloc(sizeof(int) * w);
            for (int j = 0; j < w; j++) {
                scr[i][j] = 0;
            }
        }
    }

    bool collision(figure fg, int rot, int mv_x, int mv_y) {
        for (int i = 0; i < fg.sq_sz; i++) {
            for (int j = 0; j < fg.sq_sz; j++) {
                if (!fg.scr[rot][i][j]) {
                    continue;
                }
                if (!scr[fg.ps.x + i + mv_x][fg.ps.y + j + mv_y]) {
                    continue;
                }
                return true;
            }
        }
        return false;
    }

    bool collision(figure fg, int mv_x, int mv_y) {
        return collision(fg, fg.rot, mv_x, mv_y);
    }


    bool check_fall(figure fg) {
        return !collision(fg, fg.rot, 1, 0);
    }

    bool stop(figure fg) {
        return collision(fg, fg.rot, 0, 0);
    }

    int rm_ln() {
        int ln_num = 0;
        for (int i = 0; i < h - 1; i++) {
            bool rm = true;
            for (int j = 1; j < w - 1; j++) {
                if (scr[i][j] == 0) {
                    rm = false;
                }
            }
            if (rm) {
                ln_num++;
                for (int j = i; j > 0; j--) {
                    for (int k = 1; k < w - 1; k++) {
                        scr[j][k] = scr[j - 1][k];
                    }
                }
                for (int j = 1; j < w - 1; j++) {
                    scr[0][j] = 0;
                }
            }
        }
        return ln_num;
    }

    void add(figure fg) {
        for (int i = 0; i < fg.sq_sz; i++) {
            for (int j = 0; j < fg.sq_sz; j++) {
                if (!fg.scr[fg.rot][i][j]) {
                    continue;
                }
                scr[fg.ps.x + i][fg.ps.y + j] = fg.scr[fg.rot][i][j];
            }
        }
    }

    screen(int _w, int _h) {
        build(_w, _h);
        str_w = str_h = 1;
    }
    screen(int _w, int _h, int _str_w, int _str_h) {
        build(_w, _h);
        str_w = _str_w;
        str_h = _str_h;
    }
};


void inp_prc(screen scr, figure &fg, int input) {
    if (input == 105) {
        if (!scr.collision(fg, 0, -1)) {
            fg.ps.y--;
        }
        return;
    }
    if (input == 106) {
        if (!scr.collision(fg, 0, 1)) {
            fg.ps.y++;
        }
        return;
    }
    if (input == 103) {
        if (!scr.collision(fg, (fg.rot + 1) % fg.rt_num, 0, 0)) {
            fg.rot = (fg.rot + 1) % fg.rt_num;
        }
        return;
    }
    if (input == 108) {
        if (!scr.collision(fg, 1, 0)) {
            fg.ps.x++;
        }
        return;
    }
    if (input == 57) {
        while(scr.check_fall(fg)) {
            fg.ps.x++;
        }
    }
}


void play(screen scr, int buf_input_sz, int *buf_input, int fig_num, figure *figures, const int FPS, const int NXT_LVL, const int TM_SP) {
    int level = 1;
    int frames = 0;
    int cnt_lns = 0;
    int score = 0;

    figure fg = figures[gen() % fig_num];
    figure next_fg;
    int read_pos = 0;
    while (true) {
        next_fg = figures[gen() % fig_num];
        while (true) {
            if (scr.stop(fg)) {
                return;
            }

            while (read_pos != buf_input[0]) {
                inp_prc(scr, fg, buf_input[read_pos]);
                read_pos++;
                if (read_pos == buf_input_sz) {
                    read_pos = 1;
                }
            }

            system("clear");
            scr.draw(fg, score, level, next_fg);
            std::this_thread::sleep_for(std::chrono::milliseconds(1000 / FPS));

            if (++frames < FPS / (level + TM_SP)) {
                continue;
            }

            frames = 0;
            if (!scr.check_fall(fg)) {
                scr.add(fg);
                frames = 0;
                int rm_lines = scr.rm_ln();
                cnt_lns += rm_lines != 0;
                if (rm_lines == 1) {
                    score += 40 * level;
                } else if (rm_lines == 2) {
                    score += 100 * level;
                } else if (rm_lines == 3) {
                    score += 300 * level;
                } else if (rm_lines == 4) {
                    score += 1200 * level;
                }

                if (cnt_lns >= NXT_LVL * level) {
                    level++;
                    cnt_lns = 0;
                }
                break;
            }
            fg.ps.x++;
        }
        fg = next_fg;
    }
}

int main() {
    const int FPS = 100;
    const int NXT_LVL = 10;
    const int TM_SP = 1;
    const int w = 10 + 2;
    const int h = 20 + 2;
    const int str_w = 5;
    const int str_h = 2;
    const int fig_num = 7;
    const int buf_input_sz = 100;

int *buf_input = (int *) std::malloc(sizeof(int) * buf_input_sz);
    std::thread read(read_input, buf_input_sz, buf_input);

    figure *figures = (figure *) std::malloc(sizeof(figure) * fig_num);
    for (int i = 0; i < fig_num; i++) {
        figures[i].read(w);
    }

    screen scr = screen(w, h, str_w, str_h);
    for(int i = 0; i < h; i++) {
        for (int j = 1; j < w - 1; j++) {
            scr.scr[i][j] = 0;
        }
        scr.scr[i][0] = scr.scr[i][w - 1] = 8;
    }
    for (int i = 0; i < w; i++) {
        scr.scr[h - 1][i] = 8;
    }

    play(scr, buf_input_sz, buf_input, fig_num, figures, FPS, NXT_LVL, TM_SP);
    exit(0);
    return 0;
}
