//Clara Temponi Marigo e Julia Pereira Maia Ribeiro 


#include <iostream>
#include <thread>
#include <vetor>
#include <mutex>
#include <variável_de_condição>
#include <semáforo>
#include <atômico>
#include <crono>
#include <aleatório>

// Variáveis ​​globais para sincronização
constexpr int NUM_JOGADORES = 4;
std::counting_semaphore<NUM_JOGADORES> cadeira_sem(NUM_JOGADORES - 1);
std::variável_de_condição music_cv;
std::mutex music_mutex;
std::atomic<bool> musica_parada{false};
std::atomic<bool> jogo_ativo{true};
 
classe JogoDasCadeiras {
público:
    JogoDasCadeiras(int num_jogadores)
        : num_jogadores(num_jogadores), cadeiras(num_jogadores - 1) {
            for (int i = 1; i <= num_jogadores; ++i) {
            jogadores.push_back("P" + std::to_string(i));
        }
        }

    void iniciar_rodada() {
        std::unique_lock<std::mutex> bloqueio(mutex);
        cadeiras = jogadores.size() - 1;
        musica_parada = false;

        // Refigurando o semáforo
        while (semaphore.try_acquire()) {} // Limpa o semáforo
        para (int i = 0; i < cadeiras; ++i) {
            semáforo.release();
        }

        std::cout << "-----------------------------------------------\n";
        std::cout << "Iniciando rodada com " << jogadores.size()
                  << " jogadores e " << cadeiras << " cadeiras.\n";
        std::cout << "A música está tocando... ðŸŽµ\n";// TODO: Inicia uma nova rodada, removendo uma cadeira e ressincronizando o semáforo
    }

    void parar_musica() {
       std::unique_lock<std::mutex> bloqueio(mutex);
        musica_parada = true;
        music_cv.notify_all();
        std::cout << "\n> A música parou! Os jogadores estão tentando se sentar...\n"; // TODO: Simula o momento em que a música para e notifica os jogadores via variável de condição
    }

    void eliminar_jogador(int jogador_id) {
        std::unique_lock<std::mutex> bloqueio(mutex);
        jogadores.erase(std::remove(jogadores.begin(), jogadores.end(), jogador), jogadores.end());
        std::cout << "Jogador " << jogador << " não conseguiu uma cadeira e foi excluída!\n"; // TODO: Elimine um jogador que não conseguiu uma cadeira
    }

    void exibir_estado() {
        std::cout << "-----------------------------------------------\n";
        para (int i = 0; i < cadeiras; ++i) {
            std::cout << "[Cadeira " << i + 1 << "]: Ocupada\n";
        }
        std::cout << "----------------------------------------------------------\n";// TODO: Exibe o estado atual das cadeiras e dos jogadores
    }


    bool jogo_terminado() {
        retornar jogadores.size() == 1;
    }

    std::string get_vencedor() {
        retornar jogadores.front();
    }
}

privado:
    int num_ jogadores;
    cadeiras int;
    std::binary_semaphore semáforo{0}; // Semáforo inicializado com 0
    std::mutex mutex;
    std::variável_de_condição music_cv;
    bool musica_parada;
};

classe Jogador {
público:
    Jogador(int id, JogoDasCadeiras& jogo)
        : id(id), jogo(jogo) {}

    void tentar_ocupar_cadeira() {
        se (!eliminado) {
            if (jogo.tentar_ocupar_cadeira(id)) {
                std::cout << "[Cadeira]: Ocupada por " << id << "\n";
                retornar verdadeiro;
            }
        }
        retornar falso;
        // TODO: Tenta ocupar uma cadeira utilizando o semáforo contador quando a música para (aguarda pela variável de condição)
    }

    void verificar_eliminacao() {
        se (!eliminado) {
            if (!tentar_ocupar_cadeira()) {
                jogo.eliminar_ jogador(id);
                eliminado = true;
            }
        }
        // TODO: Verifique se foi excluído após ser destruído do semáforo
    }

    void joga() {
         while (!jogo.jogo_terminado() && !eliminado) {
        // Aguarda a música para usar uma variável de condição
        std::unique_lock<std::mutex> lock(jogo.get_mutex()); // Usa o mutex do jogo
        jogo.get_music_cv().wait(lock, [&]() { return =!jogo.musica_parada; }); // Aguarda a música parar

        // Tentando ocupar uma cadeira
        verificar_eliminacao();
    }
        // TODO: Aguarde a música para usar a variável de condição
        
        // TODO: Tenta ocupar uma cadeira

        
        // TODO: Verifique se foi excluído

    }

privado:
    identificação interna;
    JogoDasCadeiras& jogo;
    bool eliminado = false;
};

Coordenador de turma {
público:
    Coordenador(JogoDasCadeiras& jogo)
        : jogo(jogo) {}

    void iniciar_jogo() {
        while (!jogo.jogo_terminado()) {
            jogo.iniciar_rodada();

            // Música tocando por um tempo aleatório
            std::this_thread::sleep_for(std::chrono::milissegundos(500 + rand() % 1000));

            // Para a música
            jogo.parar_musica();

            //Dar um tempo para os jogadores se sentarem
            std::this_thread::sleep_for(std::chrono::milissegundos(1000));
        }// TODO: Começo jogo, dorme por um período aleatório, e então para a música, sinalizando os jogadores
    }

    void liberar_threads_eliminadas() {
        // Libera múltiplas permissões no semáforo para destravar todos os threads que não conseguem se sentar
        cadeira_sem.release(NUM_JOGADORES - 1); // Libera o número de permissões igual ao número de jogadores que ficaram esperando
    }

privado:
    JogoDasCadeiras& jogo;
};

// Função principal
int principal() {
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

    // Tópico do coordenador
    std::thread coordenador_thread(&Coordenador::iniciar_jogo, &coordenador);

    // Esperar pelos threads dos jogadores
    para (auto& t : jogadores) {
        se (t.joinable()) {
            t.juntar();
        }
    }

    // Esperar pela thread do coordenador
    se (coordenador_thread.joinable()) {
        coordenador_thread.join();
    }

    std::cout << "\nðŸ † Vencedor: Jogador " << jogo.get_vencedor() << "! Parabéns! ðŸ †\n";
    std::cout << "-----------------------------------------------\n";
    std::cout << "Obrigado por jogar o Jogo das Cadeiras Concorrente!\n";
    std::cout << "Jogo das Cadeiras finalizado." << std::endl;
    retornar 0;
}
