#include <shared_asset.hpp>

using eosio::check;
using eosio::asset;
using eosio::symbol;
using eosio::symbol_code;
using eosio::require_auth;

namespace shared_asset {

shared_asset::shared_asset(name receiver, name code, eosio::datastream<const char *> ds) :
    contract(receiver, code, ds),
    tokens(receiver, receiver.value),
    config_for_token(receiver, receiver.value)
{
}

void shared_asset::setconfig(const token_config& config)
{
    require_auth(get_self());
    check(!config_for_token.exists(), "config has already been set");
    config_for_token.set(config, get_self());
}

void shared_asset::create(const name& issuer,
                          token_id id,
                          int status,
                          const std::string& lot_name,
                          const std::string& lot_description,
                          const std::string& additional_field_1,
                          const std::string& additional_field_2,
                          const std::string& additional_field_3)
{
    require_auth(get_self());

    check(config_for_token.exists(), "the configuration must be set");
    auto config = config_for_token.get();
    symbol symbol(config.symbol_code, 0);

    check(lot_name.size()           <= 256, "lot_name has more than 256 bytes");
    check(lot_description.size()    <= 256, "lot_description has more than 256 bytes");
    check(additional_field_1.size() <= 256, "additional_field_1 has more than 256 bytes");
    check(additional_field_2.size() <= 256, "additional_field_2 has more than 256 bytes");
    check(additional_field_3.size() <= 256, "additional_field_3 has more than 256 bytes");

    auto existing = tokens.find(id);
    lots_singleton lot_index(get_self(), id);
    check(!lot_index.exists(), "lot with this id has already ben created");
    check(existing == tokens.end(), "token with this id has already been created");

    tokens.emplace(get_self(), [&](auto& token) {
        token.id                 = id;
        token.issuer             = issuer;
        token.max_supply         = asset{MAX_SUPPLY_AMOUNT, symbol};
        token.supply             = asset{0, symbol};
    });

    lot lot = {
        {},                     // lot owners
        lot_name,               // lot name
        lot_description,        // lot name
        additional_field_1,     // any additional information
        additional_field_2,     // any additional information
        additional_field_3,     // any additional information
        status                  // lot status
    };

    lot_index.set(lot, get_self());
}

void shared_asset::issue(const name& to, token_id token_id, const std::string& memo)
{
    check(config_for_token.exists(), "the configuration must be set");
    auto config = config_for_token.get();
    symbol symbol(config.symbol_code, 0);
    const asset quantity(SUPPLY_AMOUNT, symbol);

    check(quantity.is_valid(), "invalid quantity");
    check(memo.size() <= 256, "memo has more than 256 bytes");

    auto existing = tokens.find(token_id);
    check(existing != tokens.end(), "token with this id does not exist, create token before issue");
    const auto& token = *existing;

    lots_singleton lot_index(get_self(), token_id);
    check(lot_index.exists(), "lot with this id does not exist, create before issue");

    require_auth(token.issuer);
    check(quantity.amount > 0, "must issue positive quantity");
    check(quantity.symbol == token.max_supply.symbol, "symbol precision mismatch");
    check(quantity.amount <= token.max_supply.amount - token.supply.amount, "quantity exceeds available supply");

    tokens.modify(token, eosio::same_payer, [&](auto& t) {
        t.supply += quantity;
    });

    auto lot = lot_index.get();
    lot.owners.push_back({to, quantity});
    lot_index.set(lot, get_self());

    add_balance(to, token_id, quantity, token.issuer);
}

void shared_asset::transfer(const name& from, const name& to, token_id token_id, const asset& quantity, const std::string& memo)
{
    require_auth(from);

    check(quantity.is_valid(), "invalid quantity");
    check(memo.size() <= 256, "memo has more than 256 bytes");

    const auto& token = tokens.get(token_id, "token with this id does not exist");
    lots_singleton lot_index(get_self(), token_id);
    check(lot_index.exists(), "lot with this id does not exist");

    check(quantity.amount > 0, "must transfer positive quantity");
    check(quantity.symbol == token.max_supply.symbol, "symbol precision mismatch");
    check(token.supply.amount >= quantity.amount, "quantity must be less than or equal to token supply");

    auto payer = has_auth(to) ? to : from;

    sub_balance(from, token_id, quantity);
    add_balance(to, token_id, quantity, payer);

    auto lot = lot_index.get();

    // update information about the owner
    auto from_it = std::find_if(lot.owners.begin(), lot.owners.end(), [&](const owner_info& owner) { 
        return from == owner.name;
    });

    // the owner has transferred all tokens. we remove it from the owners.
    if (0 == get_balance(from, token_id).amount) {
        lot.owners.erase(from_it);
    } else {
        from_it->balance -= quantity;
    }

    // update information about the new owner
    auto to_it = std::find_if(lot.owners.begin(), lot.owners.end(), [&](owner_info& owner) {
        return to == owner.name;
    });

    if (to_it == lot.owners.end()) {
        lot.owners.push_back({to, quantity});
    } else {
        to_it->balance += quantity;
    }

    lot_index.set(lot, get_self());
}

void shared_asset::add_balance(const eosio::name& owner, 
                               token_id token_id, 
                               const eosio::asset& value, 
                               const eosio::name& ram_payer)
{
    accounts_index to_acnts(get_self(), owner.value);
    auto to = to_acnts.find(token_id);

    if (to == to_acnts.end()) {
        to_acnts.emplace(ram_payer, [&](auto& a) {
            a.token_id = token_id;
            a.balance = value;
        });
    } else {
        to_acnts.modify(to, eosio::same_payer, [&](auto& a) {
            a.balance += value;
        });
    }
}

void shared_asset::sub_balance(const name& owner, token_id token_id, const eosio::asset& value)
{
    accounts_index from_acnts(get_self(), owner.value);
    const auto& from = from_acnts.get(token_id, "no balance object found");
    check(from.balance.amount >= value.amount, "overdrawn balance");

    from_acnts.modify(from, owner, [&](auto& a) {
        a.balance -= value;
    });
}

asset shared_asset::get_balance(const name& owner, token_id token_id)
{
    accounts_index from_acnts(get_self(), owner.value);
    const auto& from = from_acnts.get(token_id, "no balance object found");
    return from.balance;
}

void shared_asset::setstatus(token_id token_id, int status)
{
    const auto& token = tokens.get(token_id, "token with this id does not exist");
    require_auth(token.issuer);
    
    lots_singleton lot_index(get_self(), token_id);
    auto lot = lot_index.get();

    check(status > 0, "must be positive");

    lot.status = status;
    lot_index.set(lot, get_self());
}

void shared_asset::setlotname(token_id token_id, const std::string& lot_name)
{
    const auto& token = tokens.get(token_id, "token with this id does not exist");
    require_auth(token.issuer);

    lots_singleton lot_index(get_self(), token_id);
    auto lot = lot_index.get();

    check(lot_name.size() <= 256, "lot_name has more than 256 bytes");

    lot.lot_name = lot_name;
    lot_index.set(lot, get_self());
}

void shared_asset::setlotdesc(token_id token_id, const std::string& lot_description)
{
    const auto& token = tokens.get(token_id, "token with this id does not exist");
    require_auth(token.issuer);

    lots_singleton lot_index(get_self(), token_id);
    auto lot = lot_index.get();

    check(lot_description.size() <= 256, "lot_description has more than 256 bytes");

    lot.lot_description = lot_description;
    lot_index.set(lot, get_self());
}

void shared_asset::setaddfield1(token_id token_id, const std::string& additional_field_1)
{
    const auto& token = tokens.get(token_id, "token with this id does not exist");
    require_auth(token.issuer);

    lots_singleton lot_index(get_self(), token_id);
    auto lot = lot_index.get();

    check(additional_field_1.size() <= 256, "additional_field_1 has more than 256 bytes");

    lot.additional_field_1 = additional_field_1;
    lot_index.set(lot, get_self());
}

void shared_asset::setaddfield2(token_id token_id, const std::string& additional_field_2)
{
    const auto& token = tokens.get(token_id, "token with this id does not exist");
    require_auth(token.issuer);

    lots_singleton lot_index(get_self(), token_id);
    auto lot = lot_index.get();

    check(additional_field_2.size() <= 256, "additional_field_2 has more than 256 bytes");

    lot.additional_field_2 = additional_field_2;
    lot_index.set(lot, get_self());
}

void shared_asset::setaddfield3(token_id token_id, const std::string& additional_field_3)
{
    const auto& token = tokens.get(token_id, "token with this id does not exist");
    require_auth(token.issuer);

    lots_singleton lot_index(get_self(), token_id);
    auto lot = lot_index.get();

    check(additional_field_3.size() <= 256, "additional_field_3 has more than 256 bytes");

    lot.additional_field_3 = additional_field_3;
    lot_index.set(lot, get_self());
}

} // namespace asset
