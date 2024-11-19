#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <semaphore>
#include <atomic>
#include <chrono>
#include <random>

// Global variables for synchronization
constexpr int NUM_JOGADORES = 4;
std::counting_semaphore<NUM_JOGADORES> cadeira_sem(NUM_JOGADORES - 1); // Inicia com n-1 cadeiras, capacidade máxima n
std::condition_variable music_cv;
std::mutex music_mutex;
std::atomic<bool> musica_parada{false};
std::atomic<bool> jogo_ativo{true};

/*
 * Uso básico de um counting_semaphore em C++:
 * 
 * O `std::counting_semaphore` é um mecanismo de sincronização que permite controlar o acesso a um recurso compartilhado 
 * com um número máximo de acessos simultâneos. Neste projeto, ele é usado para gerenciar o número de cadeiras disponíveis.
 * Inicializamos o semáforo com `n - 1` para representar as cadeiras disponíveis no início do jogo. 
 * Cada jogador que tenta se sentar precisa fazer um `acquire()`, e o semáforo permite que até `n - 1` jogadores 
 * ocupem as cadeiras. Quando todos os assentos estão ocupados, jogadores adicionais ficam bloqueados até que 
 * o coordenador libere o semáforo com `release()`, sinalizando a eliminação dos jogadores.
 * O método `release()` também pode ser usado para liberar múltiplas permissões de uma só vez, por exemplo: `cadeira_sem.release(3);`,
 * o que permite destravar várias threads de uma só vez, como é feito na função `liberar_threads_eliminadas()`.
 *
 * Métodos da classe `std::counting_semaphore`:
 * 
 * 1. `acquire()`: Decrementa o contador do semáforo. Bloqueia a thread se o valor for zero.
 *    - Exemplo de uso: `cadeira_sem.acquire();` // Jogador tenta ocupar uma cadeira.
 * 
 * 2. `release(int n = 1)`: Incrementa o contador do semáforo em `n`. Pode liberar múltiplas permissões.
 *    - Exemplo de uso: `cadeira_sem.release(2);` // Libera 2 permissões simultaneamente.
 */

// Classes
class JogoDasCadeiras {
public:
    JogoDasCadeiras(int num_jogadores)
        : num_jogadores(num_jogadores), cadeiras(num_jogadores - 1) {
            for (int i = 1; i <= num_jogadores; ++i) {
            jogadores.push_back("P" + std::to_string(i));
        }
        }

    void iniciar_rodada() {
        std::unique_lock<std::mutex> lock(mutex);
        cadeiras = jogadores.size() - 1;
        musica_parada = false;

        // Reconfigurando o semáforo
        while (semaphore.try_acquire()) {} // Limpa o semáforo
        for (int i = 0; i < cadeiras; ++i) {
            semaphore.release();
        }

        std::cout << "-----------------------------------------------\n";
        std::cout << "Iniciando rodada com " << jogadores.size()
                  << " jogadores e " << cadeiras << " cadeiras.\n";
        std::cout << "A música está tocando... 🎵\n";// TODO: Inicia uma nova rodada, removendo uma cadeira e ressincronizando o semáforo
    }

    void parar_musica() {
       std::unique_lock<std::mutex> lock(mutex);
        musica_parada = true;
        music_cv.notify_all();
        std::cout << "\n> A música parou! Os jogadores estão tentando se sentar...\n"; // TODO: Simula o momento em que a música para e notifica os jogadores via variável de condição
    }

    void eliminar_jogador(int jogador_id) {
        std::unique_lock<std::mutex> lock(mutex);
        jogadores.erase(std::remove(jogadores.begin(), jogadores.end(), jogador), jogadores.end());
        std::cout << "Jogador " << jogador << " não conseguiu uma cadeira e foi eliminado!\n"; // TODO: Elimina um jogador que não conseguiu uma cadeira
    }

    void exibir_estado() {
        std::cout << "-----------------------------------------------\n";
        for (int i = 0; i < cadeiras; ++i) {
            std::cout << "[Cadeira " << i + 1 << "]: Ocupada\n";
        }
        std::cout << "-----------------------------------------------\n";// TODO: Exibe o estado atual das cadeiras e dos jogadores
    }


    bool jogo_terminado() {
        return jogadores.size() == 1;
    }

    std::string get_vencedor() {
        return jogadores.front();
    }
}

private:
    int num_jogadores;
    int cadeiras;
    std::binary_semaphore semaphore{0}; // Semáforo inicializado com 0
    std::mutex mutex;
    std::condition_variable music_cv;
    bool musica_parada;
};

class Jogador {
public:
    Jogador(int id, JogoDasCadeiras& jogo)
        : id(id), jogo(jogo) {}

    void tentar_ocupar_cadeira() {
        if (!eliminado) {
            if (jogo.tentar_ocupar_cadeira(id)) {
                std::cout << "[Cadeira]: Ocupada por " << id << "\n";
                return true;
            }
        }
        return false;
        // TODO: Tenta ocupar uma cadeira utilizando o semáforo contador quando a música para (aguarda pela variável de condição)
    }

    void verificar_eliminacao() {
        if (!eliminado) {
            if (!tentar_ocupar_cadeira()) {
                jogo.eliminar_jogador(id);
                eliminado = true;
            }
        }
        // TODO: Verifica se foi eliminado após ser destravado do semáforo
    }

    void joga() {
         while (!jogo.jogo_terminado() && !eliminado) {
        // Aguarda a música parar usando a variável de condição
        std::unique_lock<std::mutex> lock(jogo.get_mutex()); // Usa o mutex do jogo
        jogo.get_music_cv().wait(lock, [&]() { return =!jogo.musica_parada; }); // Aguarda a música parar

        // Tenta ocupar uma cadeira
        verificar_eliminacao();
    }
        // TODO: Aguarda a música parar usando a variavel de condicao
        
        // TODO: Tenta ocupar uma cadeira

        
        // TODO: Verifica se foi eliminado

    }

private:
    int id;
    JogoDasCadeiras& jogo;
    bool eliminado = false;
};

class Coordenador {
public:
    Coordenador(JogoDasCadeiras& jogo)
        : jogo(jogo) {}

    void iniciar_jogo() {
        while (!jogo.jogo_terminado()) {
            jogo.iniciar_rodada();

            // Música tocando por um tempo aleatório
            std::this_thread::sleep_for(std::chrono::milliseconds(500 + rand() % 1000));

            // Parar a música
            jogo.parar_musica();

            // Dar um tempo para os jogadores se sentarem
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }// TODO: Começa o jogo, dorme por um período aleatório, e então para a música, sinalizando os jogadores 
    }

    void liberar_threads_eliminadas() {
        // Libera múltiplas permissões no semáforo para destravar todas as threads que não conseguiram se sentar
        cadeira_sem.release(NUM_JOGADORES - 1); // Libera o número de permissões igual ao número de jogadores que ficaram esperando
    }

private:
    JogoDasCadeiras& jogo;
};

// Main function
int main() {
    JogoDasCadeiras jogo(NUM_JOGADORES);
    Coordenador coordenador(jogo);
    std::vector<std::thread> jogadores;

    // Criação das threads dos jogadores
    std::vector<Jogador> jogadores_objs;
    for (int i = 1; i <= NUM_JOGADORES; ++i) {
        jogadores_objs.emplace_back(i, jogo);
    }

    for (int i = 0; i < NUM_JOGADORES; ++i) {
        jogadores.emplace_back(&Jogador::joga, &jogadores_objs[i]);
    }

    // Thread do coordenador
    std::thread coordenador_thread(&Coordenador::iniciar_jogo, &coordenador);

    // Esperar pelas threads dos jogadores
    for (auto& t : jogadores) {
        if (t.joinable()) {
            t.join();
        }
    }

    // Esperar pela thread do coordenador
    if (coordenador_thread.joinable()) {
        coordenador_thread.join();
    }

    std::cout << "\n🏆 Vencedor: Jogador " << jogo.get_vencedor() << "! Parabéns! 🏆\n";
    std::cout << "-----------------------------------------------\n";
    std::cout << "Obrigado por jogar o Jogo das Cadeiras Concorrente!\n";
    std::cout << "Jogo das Cadeiras finalizado." << std::endl;
    return 0;
}
  
