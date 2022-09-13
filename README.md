![Logo](https://i.postimg.cc/xCtC038W/ilink.png)

# Shared Asset contract

## Build and Run
You can use the docker image to build contracts.
All build artifacts will be stored in `./build/contracts`.

```shell
docker build -t shared.asset .
docker run -it -v $(pwd):/project/contracts shared.asset
. build.sh
```

To run the eos node, you can use the same docker image and play 
around with the deployed contract.

```shell
docker build -t shared.asset .
docker run -it -v $(pwd):/project/contracts -p 8888:8888 -p 9876:9876 shared.asset
. init.sh
```

## Actions
### setconfig
Sets the base token settings such as the symbol for the asset.
```shell
cleos push action shared.asset setconfig \
    '{
        "config": {
            "symbol_code":"TEZRO"
        }
    }' -p shared.asset@active
```

### create
Allows `issuer` account to create a token in supply of `maximum_supply`. 
If validation is successful a new entry in tokens table for token gets 
created. Called only after calling setconfig.

```shell
cleos push action shared.asset create \
    '{
        "issuer": "shared.asset",
        "id": 1,
        "status": 0,
        "lot_name": "lot.1",
        "lot_description": "lot.description",
        "additional_field_1": "additional.field.1",
        "additional_field_2": "additional.field.2",
        "additional_field_3": "additional.field.3"
    }' -p shared.asset@active
```

### issue
This action issues to `to` account a `quantity` of tokens. Called only by
`issuer` account that was specified when calling create.

```shell
cleos push action shared.asset issue \
    '{
        "to": "alice",
        "token_id": 0,
        "memo": "some memo"
    }' -p shared.asset@active
```

### transfer
Allows `from` account to transfer to `to` account the `quantity` tokens.
One account is debited and the other is credited with quantity tokens.

```shell
cleos push action shared.asset transfer \
    '{
        "from": "alice",
        "to": "bob",
        "token_id": 0,
        "quantity": "50 TEZRO",
        "memo": "some memo"
    }' -p alice@active
```

### other actions
You can find a full description of all actions, with checks and other things
[`./contracts/asset/include/shared_asset.hpp`](./contracts/asset/include/shared_asset.hpp)
