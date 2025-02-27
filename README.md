# Timer Configurável com Pausa
Bem-vindo ao repositório do "Timer Configurável com Pausa", um sistema embarcado desenvolvido para a plataforma BitDogLab, baseado no Raspberry Pi Pico. Este projeto foi criado como parte do programa EmbarcaTech, demonstrando a aplicação de conceitos de Sistemas Embarcados (SE) em uma solução original e funcional.

# Vídeo de demonstração
https://youtu.be/uTj-hAmcuU4?si=4E8655NsW7FiM0gD

## Descrição
O "Timer Configurável com Pausa" é um temporizador versátil que permite ao usuário definir um tempo em minutos, iniciar a contagem regressiva, pausá-la e receber notificações visuais e sonoras ao final. O sistema utiliza a BitDogLab para integrar interfaces de entrada (joystick e botões) e saída (display OLED, matriz de LEDs e buzzers), oferecendo uma experiência interativa e intuitiva.

### Funcionalidades
- **Configuração do Tempo**: Ajuste o tempo em minutos usando o eixo Y do joystick.
- **Contagem Regressiva**: Exibe o tempo restante no display SSD1306 em segundos.
- **Pausa e Retomada**: Pausa a contagem ao entrar no modo de configuração e retoma ao sair.
- **Contagem Visual**: Mostra os últimos 10 segundos em uma matriz de LEDs 5x5.
- **Alerta Sonoro**: Emite cinco bips de 200 ms via buzzers ao finalizar.
- **Mensagem de Término**: Exibe "Timer encerrado" apenas no modo de execução.

## Requisitos

### Hardware
- **BitDogLab** com Raspberry Pi Pico.
- Componentes integrados:
  - Joystick (eixo Y e botão).
  - Botões A e B.
  - Display SSD1306 (OLED, 128x64).
  - Matriz de LEDs 5x5 (WS2812).
  - Dois buzzers.
  - LEDs RGB (opcional, não utilizados).

### Software
- **Pico SDK**: Versão 2.1.0.
- **Ambiente de Desenvolvimento**: VS Code ou semelhante, com suporte ao Pico SDK.
- **Linguagem**: C.

## Instruções de Uso

1. **Configuração**:
   - Ligue a BitDogLab.
   - Use o eixo Y do joystick para ajustar o tempo em minutos (ex.: 1 a 5 minutos).

2. **Operação**:
   - Pressione o botão B para alternar entre modo de configuração (`configuracao = true`) e execução (`configuracao = false`).
   - No modo de execução, pressione o botão A para iniciar ou reiniciar o temporizador.
   - Pressione o botão do joystick para ligar/desligar o sistema (`estado_sistema`).

3. **Pausa**:
   - No modo de execução, pressione o botão B para pausar (entra no modo de configuração).
   - Pressione o botão B novamente para retomar.

4. **Término**:
   - Ao atingir zero, os buzzers emitem cinco bips, e "Timer encerrado" é exibido (se em modo de execução).

## Detalhes Técnicos

### Hardware
- **Pinos Utilizados**:
  - GPIO 5: Botão A.
  - GPIO 6: Botão B.
  - GPIO 7: Matriz de LEDs (PIO).
  - GPIO 10: Buzzer 1 (PWM).
  - GPIO 11-13: LEDs RGB (saídas).
  - GPIO 14-15: I2C (SDA e SCL do SSD1306).
  - GPIO 21: Buzzer 2 (PWM).
  - GPIO 22: Botão do Joystick.
  - GPIO 27: Eixo Y do Joystick (ADC).

- **Configuração**:
  - I2C a 400 kHz para o SSD1306.
  - PIO a 800 kHz para WS2812.
  - PWM a 2700 Hz com 90% duty cycle para buzzers.

### Software
- **Estrutura**:
  - **Inicialização**: Configura GPIO, I2C, PIO, ADC e PWM.
  - **Interrupções**: Gerencia botões e joystick via `gpio_irq_handler`.
  - **Temporização**: Calcula o tempo restante com `verificar_tempo`, suporta pausa via `tempo_pausado`.
  - **Exibição**: `display_sistema` (linha superior) e `exibir_tempo` (linha inferior).
  - **Matriz de LEDs**: `display_numeros` para contagem regressiva.
  - **Buzzers**: `atualizar_alarme` para cinco bips.

- **Variáveis Principais**:
  - `contador`: Tempo configurado (minutos).
  - `tempo_inicio`: Timestamp de início (µs).
  - `tempo_rodando`: Estado do temporizador.
  - `tempo_pausado`: Tempo acumulado ao pausar (s).
  - `configuracao`: Modo do sistema.
  - `ultimo_buffer`: Último valor exibido.

## Instalação

1. **Clone o Repositório**:
   ```bash
   git clone https://github.com/seu_usuario/timer-configuravel-com-pausa.git
   cd timer-configuravel-com-pausa
2. **Configure o Pico SDK**:
  - Baixe o Pico SDK (versão 2.1.0) em https://github.com/raspberrypi/pico-sdk.
  - Siga as instruções em https://datasheets.raspberrypi.com/pico/raspberry-pi-pico-c-sdk.pdf.
4. **Compile e Carregue**:
  - Abra o projeto no VS Code com Pico SDK configurado.
  - Execute:
    ```bash
    mkdir build
    cd build
    cmake ..
    make
  - Carregue o arquivo .uf2 gerado no Pico via modo BOOTSEL.
5. **Conecte o Hardware**:
    Conecte todos os componentes à BitDogLab conforme a pinagem descrita.
