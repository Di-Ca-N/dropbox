# Dropbox
Sistema simples de sincronização de arquivos desenvolvido para a cadeira Sistemas Operacionais II.

O sistema suporta sincronização de arquivos entre dispositivos de usuários diferentes, e implementa mecanismos de replicação
passiva e algoritmo de eleição em anel para suportar falhas do servidor primário.

## Compilação
Para compilar o programa, rode os seguintes comandos

```
mkdir build
cd build
cmake ..
make
```

Os binários compilados estarão na pasta `/bin`.

## Modo de uso
Primeiro, inicie o nameserver com o comando a seguir 

```
binding-agent <port-client> <port-server>
```

Depois, inicie o servidor primário

```
server <port> <ns-ip> <ns-server-port>
```

E então as réplicas
```
server <port> <ns-ip> <ns-server-port> <primary-ip> <primary-port> <id>
```

Por fim, você pode inicializar os clientes:
```
client <username> <ns-ip> <ns-client-port>
```


