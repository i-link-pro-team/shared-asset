FROM buildpack-deps:18.04

RUN apt-get update; \
    apt-get install -y --no-install-recommends \
        wget \
        make \
        cmake \
        libusb-1.0-0 \
        libcurl3-gnutls \
    ; \
    rm -rf /var/lib/apt/lists/*

RUN wget --no-check-certificate --quiet https://github.com/EOSIO/eos/releases/download/v2.0.0/eosio_2.0.0-1-ubuntu-18.04_amd64.deb; \
    apt-get install ./eosio_2.0.0-1-ubuntu-18.04_amd64.deb

RUN wget --no-check-certificate --quiet https://github.com/EOSIO/eosio.cdt/releases/download/v1.7.0/eosio.cdt_1.7.0-1-ubuntu-18.04_amd64.deb; \
    apt-get install ./eosio.cdt_1.7.0-1-ubuntu-18.04_amd64.deb

RUN mkdir /project

# private develop key 5KQwrPbwdL6PhXujxW37FSSQZ1JiwsST4cqQzDeyXtP79zkvFD3
# public develop key EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
RUN cleos wallet create -f /project/password && \
    cleos wallet open && \
    cat /project/password | cleos wallet unlock && \
    cleos wallet create_key | awk 'match($0, /"(.*)"/) { print substr( $0, RSTART + 1, RLENGTH - 2)}' >> /project/pub.key && \
    echo "5KQwrPbwdL6PhXujxW37FSSQZ1JiwsST4cqQzDeyXtP79zkvFD3" | cleos wallet import

WORKDIR /project/contracts

VOLUME [ "/project/contracts" ]

EXPOSE 8888
EXPOSE 9876

CMD nodeos -e -p eosio \
    --plugin eosio::producer_plugin \
    --plugin eosio::producer_api_plugin \
    --plugin eosio::chain_api_plugin \
    --plugin eosio::http_plugin \
    --plugin eosio::history_plugin \
    --plugin eosio::history_api_plugin \
    --filter-on="*" \
    --access-control-allow-origin='*' \
    --contracts-console \
    --http-validate-host=false \
    --http-server-address 0.0.0.0:8888 \
    --verbose-http-errors >> ../nodeos.log 2>&1 & \
    bash
