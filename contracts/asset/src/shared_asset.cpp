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
    check(!config_for_token.exists(), "config has already been set");
}

void shared_asset::create(const name& issuer, const std::string& token_name)
{
    require_auth(get_self());

    auto config = config_for_token.get();
    symbol symbol(config.symbol_code, 0);

    tokens.emplace(get_self(), [&](auto& token) {
        token.id            = tokens.available_primary_key();
        token.issuer        = issuer;
        token.token_name    = token_name;
        token.max_supply    = asset{100, symbol};
        token.supply        = asset{0, symbol};
    });
}

void shared_asset::issue(const name& to, const token_id& token_id, const eosio::asset& quantity, const std::string& memo)
{
    check(quantity.symbol.is_valid(), "invalid symbol name");
    check(memo.size() <= 256, "memo has more than 265 bytes");

    auto existing = tokens.find(token_id);
    check(existing != tokens.end(), "token with this id does not exist, create token before issue");
    const auto& token = *existing;

    require_auth(token.issuer);
    check(quantity.is_valid(), "invalid quantity");
    check(quantity.amount > 0, "must issue positive quantity");
}

void shared_asset::add_balance(const eosio::name& owner, const eosio::asset& value)
{

}

void shared_asset::sub_balance(const eosio::name& owner, const eosio::asset& value)
{

}

} // namespace asset
