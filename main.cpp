#include <SFML/Graphics.hpp>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <time.h>

//TO DO: use drawflag

using namespace std;

const int SCALE = 10;

const int WIDTH = 64;
const int HEIGHT = 32;

sf::RenderWindow window(sf::VideoMode(WIDTH* SCALE, HEIGHT* SCALE), "Chip-8");

class chip8
{
public:
    unsigned short opcode={};
    unsigned char memory[4096]={};
    unsigned char V[16]={};
    unsigned short indx={};
    unsigned short pc={};
    bool graphics[WIDTH * HEIGHT] = {};

    unsigned char delayT={};
    unsigned char soundT={};
    unsigned short stack[16]={};
    unsigned short stackPointer={};
    unsigned char key[16]={};


    unsigned char chip8_fontset[80] =
    {
      0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
      0x20, 0x60, 0x20, 0x20, 0x70, // 1
      0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
      0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
      0x90, 0x90, 0xF0, 0x10, 0x10, // 4
      0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
      0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
      0xF0, 0x10, 0x20, 0x40, 0x40, // 7
      0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
      0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
      0xF0, 0x90, 0xF0, 0x90, 0x90, // A
      0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
      0xF0, 0x80, 0x80, 0x80, 0xF0, // C
      0xE0, 0x90, 0x90, 0x90, 0xE0, // D
      0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
      0xF0, 0x80, 0xF0, 0x80, 0x80  // F
    };

    bool drawFlag = {};
    bool keyPressed = {};
    bool awaitKey = {};
    unsigned char keyAwaited = {};

    void setup()
    {

      pc = 0x200;
      opcode = 0;
      indx = 0;
      stackPointer = 0;

      // load fontset
      for (int i = 0; i < 80; i++) {
        memory[i] = chip8_fontset[i];
      }
    }

    void awaitKeyPress(unsigned short X)
    {
      awaitKey = true;
      if (keyPressed) {
        V[X] = keyAwaited;
        pc += 2;
        keyPressed = false;
        awaitKey = false;
      }
    }
    void skipNextInst()
    {
      // todo
      pc += 4;
    }
    void load()
    {
      std::ifstream input("C:\\CH8testroms\\AdditionGame.ch8",
                          std::ios::binary);
      if (!input) {
        printf("failed to open file");
      }
      input.read((char*)(memory + 0x200), 4096 - 0x200);
    }

    void cycle()
    {
      pc %= 4096;
      stackPointer %= 16;
      indx %= 4096;

      // get opcode
      opcode = memory[pc] << 8 | memory[(pc + 1) % 4096];

      unsigned short Y = (opcode & 0x00F0) >> 4;
      unsigned short X = (opcode & 0x0F00) >> 8; // 0XY0;

      // check if any key has been pressed

      // decode
      switch (opcode & 0xF000) {
        // do op codes that are dependant on 1st bit
        case 0x1000: // 1NNN
          pc = opcode & 0x0FFF;
          break;
        case 0x2000: // 2NNN
          pc += 2;
          ++stackPointer;
          stackPointer %= 16;
          stack[stackPointer] = pc;
          pc = opcode & 0x0FFF;
          break;
        case 0x3000: // 3XNN
          if (V[X] == (opcode & 0x00FF)) {
            skipNextInst();
          } else {
            pc += 2;
          }
          break;
        case 0x4000: // 4XNN
          if (V[X] != (opcode & 0x00FF)) {
            skipNextInst();
          } else {
            pc += 2;
          }
          break;
        case 0x5000: // 5XY0
          if (V[X] == V[Y]) {
            skipNextInst();
          } else {
            pc += 2;
          }
          break;
        case 0x6000: // 6XNN
          V[X] = opcode & 0x00FF;
          pc += 2;
          break;
        case 0x7000: // 7XNN
          V[X] += opcode & 0x0FF;
          pc += 2;
          break;
        case 0x9000: // 9XY0
          if (V[X] != V[Y]) {
            skipNextInst();
          } else {
            pc += 2;
          }
          break;
        case 0xA000: // ANNN
          indx = opcode & 0x0FFF;
          pc += 2;
          break;

        case 0xB000: // BNNN
          pc = (opcode & 0x0FFF) + V[0];
          break;
        case 0xC000: // CXNN
          V[X] = (rand() % 256) & (opcode & 0x00FF);
          pc += 2;
          break;
        case 0xD000: // DXYN DRAWING
          this->draw(V[X], V[Y], opcode & 0x000F);

          pc += 2;
          break;

          // op codes that are dependant on other bits

        case 0x0000:
          switch (opcode & 0x000F) {
            case 0x0000: // 0x00E0
              std::fill_n(graphics, WIDTH * HEIGHT, 0);
              drawFlag = true;
              pc += 2;
              break;
            case 0x000E: // 0x0EE

              pc = stack[stackPointer];
              --stackPointer;
              break;

            default:
              printf("Unkown opcode %X", opcode);
          }
          break;

        case 0x8000:
          switch (opcode & 0x000F) {
            case 0x0000: // 8XY0
              V[X] = V[Y];
              pc += 2;
              break;
            case 0x0001: // 8XY1
              V[X] = V[X] | V[Y];
              pc += 2;
              break;
            case 0x0002: // 8XY2
              V[X] = V[X] & V[Y];
              pc += 2;
              break;
            case 0x0003: // 8XY3
              V[X] = V[X] ^ V[Y];
              pc += 2;
              break;
            case 0x0004: // 8XY4
              V[0xF] = (V[X] + V[Y] > 255)
                         ? 1
                         : 0; // if sum is greater than 255 then there is carry
              V[X] += V[Y];
              pc += 2;
              break;
            case 0x0005: // 8XY5
              V[0xF] = V[X] > V[Y];
              V[X] -= V[Y];
              pc += 2;
              break;
            case 0x0006: // 8XY6
              V[0xF] = V[X] & 1;
              V[X] >>= 1;
              pc += 2;
              break;
            case 0x0007:                      // 8XY7
              V[0xF] = (V[Y] > V[X]) ? 1 : 0; // check for borrow
              V[X] = V[Y] - V[X];
              pc += 2;
              break;
            case 0x000E: // 8XYE
              V[0xF] = (V[X] >> 7) & 1;
              V[X] <<= 1;
              pc += 2;
              break;
            default:
              printf("Unkown opcode %X", opcode);
          }
          break;

        case 0xE000:
          switch (opcode & 0x00FF) {
            case 0x009E: // EX9E
              if (key[V[X] % 16] != 0) {
                skipNextInst();
              } else {
                pc += 2;
              }
              break;
            case 0x00A1: // EXA1
              if (key[V[X] % 16] == 0) {
                skipNextInst();
              } else {
                pc += 2;
              }
              break;
            default:
              printf("Unkown opcode %X", opcode);
          }
          break;

        case 0xF000:
          switch (opcode & 0x00FF) {
            case 0x0007: // FX07
              V[X] = delayT;
              pc += 2;
              break;
            case 0x000A: // FX0A await keypress
              this->awaitKeyPress(X);
              break;
            case 0x0015: // FX15
              delayT = V[X];
              pc += 2;
              break;
            case 0x0018: // FX18
              soundT = V[X];
              pc += 2;
              break;
            case 0x001E: // FX1E
              indx += V[X];
              pc += 2;
              break;
            case 0x0029: // FX29
              printf("using fx29\n");
              indx = V[X] * 5;
              pc += 2;
              break;
            case 0x0033: // FX33 BCD
              printf("using BCD");
              memory[indx] = V[X] / 100;
              memory[(indx + 1) % 4096] = (V[X] / 10) % 10;
              memory[(indx + 2) % 4096] = V[X] % 10;
              pc += 2;
              break;
            case 0x0055: // FX55
              for (int i = 0; i <= X; i++) {

                memory[(indx + i) % 4096] = V[i];
              }
              pc += 2;
              break;
            case 0x0065: // FX65
              for (int i = 0; i <= X; i++) {
                V[i] = memory[(indx + i) % 4096];
              }
              pc += 2;
              break;
            default:
              printf("Unkown opcode %X", opcode);
          }

          break;
        default:
          printf("Unkown opcode %X", opcode);
      }

      // timers
      if (!awaitKey) {
        if (delayT > 0)
          --delayT;
        if (soundT == 1) {
          // beep
          --soundT;
        }
      }
      std::fill_n(key, 16, 0);
    }

    void draw(int x, int y, int h)
    {
      //      printf("x = %04x y = %04x h = %04x i = %06x\n", x, y, h, indx);
      //      for (int i = 0; i < h; i++) {
      //        for (int j = 0; j < 8; j++) {
      //          bool nexPix = memory[(indx * h + i) % 4096] & (0x80 >> j);
      //          if (nexPix == 0) {
      //            printf(" ");
      //          } else {
      //            printf("*");
      //          }
      //        }
      //        printf("\n");
      //      }

      V[0xF] = 0;
      for (int i = 0; i < h; i++) {
        for (int j = 0; j < 8; j++) {
          unsigned int index = ((i + y) % HEIGHT) * WIDTH + ((x + j) % WIDTH);
          bool curPix = graphics[index];
          bool nexPix = memory[(indx + i) % 4096] & (0x80 >> j);

          if (nexPix != 0) {
            if (curPix == 1) {
              V[0xF] = 1;
            }
            graphics[index] ^= nexPix;
          }
        }
      }
      drawFlag = true;
    }

    void display()
    {

      for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
          unsigned char curPix = graphics[y * WIDTH + x]; // pixel to display
          if (curPix) {
            sf::RectangleShape pixel(sf::Vector2f(SCALE, SCALE));
            pixel.setPosition(x * SCALE, y * SCALE);
            window.draw(pixel);
          }
        }
      }
    }

    void getKeyPress();
};

int
main()
{

  chip8 Chip8;
  srand(time(0));
  // graphics and input

  // setup
  Chip8.setup();
  Chip8.load();
  Chip8.key[0] = 1;
  window.clear(sf::Color::Black);
  while (window.isOpen()) {

    Chip8.cycle();
    sf::Event event;
    while (window.pollEvent(event)) {
      switch (event.type) {
        case sf::Event::Closed:
          window.close();
          break;

        case sf::Event::KeyPressed: // KEY INPUTS
          Chip8.keyPressed = true;
          switch (event.key.code) {
            case sf::Keyboard::Numpad0:
              Chip8.key[0] = 1;
              break;
            case sf::Keyboard::Numpad1:
              Chip8.key[1] = 1;
              break;
            case sf::Keyboard::Numpad2:
              Chip8.key[2] = 1;
              break;
            case sf::Keyboard::Numpad3:
              Chip8.key[0x3] = 1;
              break;
            case sf::Keyboard::Numpad4:
              Chip8.key[0x4] = 1;
              break;
            case sf::Keyboard::Numpad5:
              Chip8.key[0x5] = 1;
              break;
            case sf::Keyboard::Numpad6:
              Chip8.key[0x6] = 1;
              break;
            case sf::Keyboard::Numpad7:
              Chip8.key[0x7] = 1;
              break;
            case sf::Keyboard::Numpad8:
              Chip8.key[0x8] = 1;
              break;
            case sf::Keyboard::Numpad9:
              Chip8.key[0x9] = 1;
              break;
            case sf::Keyboard::A:
              Chip8.key[0xA] = 1;
              break;
            case sf::Keyboard::B:
              Chip8.key[0xB] = 1;
              break;
            case sf::Keyboard::C:
              Chip8.key[0xC] = 1;
              break;
            case sf::Keyboard::D:
              Chip8.key[0xD] = 1;
              break;
            case sf::Keyboard::E:
              Chip8.key[0xE] = 1;
              break;
            case sf::Keyboard::F:
              Chip8.key[0xF] = 1;
              break;

            default:

              break;
          }
          break;
      }
    }

    if (Chip8.drawFlag) {
      window.clear(sf::Color::Black);
      Chip8.display();
      window.display();
      Chip8.drawFlag = false;
    }
    sf::sleep(sf::milliseconds(1));
  }

  return 0;
}
