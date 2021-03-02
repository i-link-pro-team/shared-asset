#!/bin/bash

mkdir build 
cd build
cmake ..
make
cd -

cat /project/password | cleos wallet unlock

cleos create account eosio shared.asset $(cat /project/pub.key)
cleos create account eosio alice $(cat /project/pub.key)
cleos create account eosio bob $(cat /project/pub.key)

cleos set contract shared.asset ./build/contracts/asset/ shared_asset.wasm shared_asset.abi

cleos push action shared.asset setconfig '{"config": {"symbol_code":"TEZRO"}}' -p shared.asset@active

cleos push action shared.asset create '["shared.asset", 0, "lot.1", "lot.description", "additional.field.1", "additional.field.2", "additional.field.3"]' -p shared.asset@active
cleos push action shared.asset create '["shared.asset", 0, "lot.2", "lot.description", "additional.field.1", "additional.field.2", "additional.field.3"]' -p shared.asset@active

cleos get table shared.asset shared.asset tokens

cleos push action shared.asset issue '["alice", 0, "lot:12"]' -p shared.asset@active
cleos push action shared.asset issue '["alice", 1, "lot:13"]' -p shared.asset@active

cleos get table shared.asset shared.asset tokens

cleos push action shared.asset transfer '["alice", "bob", 0, "50 TEZRO", ""]' -p alice@active
cleos push action shared.asset transfer '["alice", "bob", 1, "1 TEZRO", ""]' -p alice@active
