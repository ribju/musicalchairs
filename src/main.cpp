#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <semaphore>
#include <random>
#include <chrono>

class JogoDasCadeiras {
private:
    int cadeiras;
    std::vector<std::string> jogadores;
    std::binary_semaphore semaphore{0}; // Semáforo inicializado com 0
    std::mutex mutex;
    std::condition_variable music_cv;
    bool musica_tocando;

public:
    JogoDasCadeiras(int num_jogadores)
        : cadeiras(num_jogadores - 1), musica_tocando(true) {
        for (int i = 1; i <= num_jogadores; ++i) {
            jogadores.push_back("P" + std::to_string(i));
        }
    }

    void iniciar_rodada() {
        std::unique_lock<std::mutex> lock(mutex);
        cadeiras = jogadores.size() - 1;
        musica_tocando = true;

        // Reconfigurando o semáforo
        while (semaphore.try_acquire()) {} // Limpa o semáforo
        for (int i = 0; i < cadeiras; ++i) {
            semaphore.release();
        }

        std::cout << "-----------------------------------------------\n";
        std::cout << "Iniciando rodada com " << jogadores.size()
                  << " jogadores e " << cadeiras << " cadeiras.\n";
        std::cout << "A música está tocando... 🎵\n";
    }

    void parar_musica() {
        std::unique_lock<std::mutex> lock(mutex);
        musica_tocando = false;
        music_cv.notify_all();
        std::cout << "\n> A música parou! Os jogadores estão tentando se sentar...\n";
    }

    bool tentar_ocupar_cadeira(const std::string& jogador) {
        music_cv.wait(mutex, [&]() { return !musica_tocando; });
        return semaphore.try_acquire();
    }

    void eliminar_jogador(const std::string& jogador) {
        std::unique_lock<std::mutex> lock(mutex);
        jogadores.erase(std::remove(jogadores.begin(), jogadores.end(), jogador), jogadores.end());
        std::cout << "Jogador " << jogador << " não conseguiu uma cadeira e foi eliminado!\n";
    }

    void exibir_estado() {
        std::cout << "-----------------------------------------------\n";
        for (int i = 0; i < cadeiras; ++i) {
            std::cout << "[Cadeira " << i + 1 << "]: Ocupada\n";
        }
        std::cout << "-----------------------------------------------\n";
    }

    bool jogo_terminado() {
        return jogadores.size() == 1;
    }

    std::string get_vencedor() {
        return jogadores.front();
    }
};

class Jogador {
private:
    std::string id;
    JogoDasCadeiras& jogo;

public:
    Jogador(std::string id, JogoDasCadeiras& jogo) : id(id), jogo(jogo) {}

    void operator()() {
        while (!jogo.jogo_terminado()) {
            if (!jogo.tentar_ocupar_cadeira(id)) {
                jogo.eliminar_jogador(id);
                break;
            }
        }
    }
};

class Coordenador {
private:
    JogoDasCadeiras& jogo;

public:
    Coordenador(JogoDasCadeiras& jogo) : jogo(jogo) {}

    void operator()() {
        while (!jogo.jogo_terminado()) {
            jogo.iniciar_rodada();

            // Música tocando por um tempo aleatório
            std::this_thread::sleep_for(std::chrono::milliseconds(500 + rand() % 1000));

            // Parar a música
            jogo.parar_musica();

            // Dar um tempo para os jogadores se sentarem
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
    }
};

int main() {
    int num_jogadores = 4;
    JogoDasCadeiras jogo(num_jogadores);

    // Criar threads dos jogadores
    std::vector<std::thread> threads;
    for (int i = 1; i <= num_jogadores; ++i) {
        threads.emplace_back(Jogador("P" + std::to_string(i), jogo));
    }

    // Thread do coordenador
    std::thread coordenador(Coordenador(jogo));

    // Aguardar término das threads
    for (auto& t : threads) {
        t.join();
    }
    coordenador.join();

    // Exibir vencedor
    std::cout << "\n🏆 Vencedor: Jogador " << jogo.get_vencedor() << "! Parabéns! 🏆\n";
    std::cout << "-----------------------------------------------\n";
    std::cout << "Obrigado por jogar o Jogo das Cadeiras Concorrente!\n";
    return 0;
}
