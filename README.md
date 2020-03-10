## Client-Srever-queue

# Compilation

```bash
> gcc -Wall serwer.c -o serwer
> gcc -Wall klient.c -o klient
```

# Start-up

```bash
> ./serwer
> ./klient nr name opt
```

nr - queue key, e.g. 123

name - client name

opt - 0 or 1 where 0 - asynchronous receiving. 1 - synchronous receiving

Example:

```bash
> ./klient 123 Michael 0
```

More info:


<a href="https://github.com/NaIwo/Client-server-queue/blob/master/PROTOCOL">Protocol</a>

