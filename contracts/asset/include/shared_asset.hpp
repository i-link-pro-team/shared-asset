#pragma once

#include <string>

#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/singleton.hpp>


namespace shared_asset {

using eosio::name;

using token_id = uint64_t;

class [[eosio::contract("shared_asset")]] shared_asset : public eosio::contract 
{
public:

   using eosio::contract::contract;

   struct token_config;

   shared_asset(name receiver, name code, eosio::datastream<const char *> ds);

   [[eosio::action]]
   void setconfig(const token_config& config);

   [[eosio::action]]
   void create(const name& issuer, const std::string& token_name);

   [[eosio::action]]
   void issue(const name& to, const token_id& token_id, const eosio::asset& quantity, const std::string& memo);

   struct [[eosio::table]] token_config
   {
      eosio::symbol_code symbol_code;
   };
   
   // scope is self
   struct [[eosio::table]] token
   {
      eosio::asset   supply;     // current supply
      eosio::asset   max_supply; // max supply
      std::string    token_name; // TODO: add secondary index
      name           issuer;     // user who can manage token
      token_id       id;         // token id

      uint64_t primary_key() const { return id; }
   };

   // scope is owner
   struct [[eosio::table]] account
   {
      eosio::asset   amount;
      token_id       token_id;

      uint64_t primary_key() const { return token_id; }
   };

private:

   typedef eosio::singleton<"tokenconfig"_n, token_config> token_config_singleton;
   typedef eosio::multi_index<"tokens"_n, token> tokens_index;
   typedef eosio::multi_index<"accounts"_n, account> accounts_index;

   token_config_singleton config_for_token;
   tokens_index tokens;

   void add_balance(const name& owner, const eosio::asset& value);
   void sub_balance(const name& owner, const eosio::asset& value);
};

} // namespace shared_asset
