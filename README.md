# 簡易サーバー実装集
CSAPPに記載している簡単なechoサーバーとwebサーバーを写経しました。

## 動かし方
各ディレクトリで`make run`を実行すると、80ポートをリッスンしているサーバーが起動します。

## 各ディレクトリの説明
以下の順に動作を試すことをお勧めします。適当なステップ毎に実装されているので、コア機能を理解しやすいと思います。

### echo_serv
echoサーバーを実装しています。

### echo_with_select
selectを用いて標準入力も受け取れるechoサーバーを実装しています。I/O多重化の概念をつかめます。

### echo_with_select@v2
selectを用いてI/O多重化したechoサーバーを実装しています。しかし、サーバーのルーチンに`echo`関数を使い、ソケットを読み込み続けると他のクライアントと通信ができず、I/O多重化による凄みを感じにくいです。

### echo_with_epoll
epollを用いてI/O多重化したechoサーバーを実装しています。selectとは異なり、ルーチンに`echo`関数を使っても他のクライアントと通信ができます。理屈がわかったら教えてください。

### simple_web_serv
簡単なwebサーバーの実装です。CGIまで対応しています。

### web_serv
I/O多重化したwebサーバーの実装です。(TBD)
