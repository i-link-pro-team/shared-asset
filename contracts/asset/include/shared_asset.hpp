#pragma once

#include <string>

#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/singleton.hpp>


namespace shared_asset {

using eosio::name;
using eosio::asset;

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
   void create(const name& issuer, 
               int status,
               const std::string& lot_name,
               const std::string& lot_description,
               const std::string& additional_field_1,
               const std::string& additional_field_2,
               const std::string& additional_field_3);

   [[eosio::action]]
   void issue(const name& to, token_id token_id, const std::string& memo);

   [[eosio::action]]
   void transfer(const name& from, const name& to, token_id token_id, const asset& quantity, const std::string& memo);

   [[eosio::action]]
   void setstatus(token_id token_id, int status);

   [[eosio::action]]
   void setlotname(token_id token_id, const std::string& lot_name);

   [[eosio::action]]
   void setlotdesc(token_id token_id, const std::string& lot_description);

   [[eosio::action]]
   void setaddfield1(token_id token_id, const std::string& additional_field_1);

   [[eosio::action]]
   void setaddfield2(token_id token_id, const std::string& additional_field_2);

   [[eosio::action]]
   void setaddfield3(token_id token_id, const std::string& additional_field_3);

   struct [[eosio::table("token.config")]] token_config
   {
      eosio::symbol_code symbol_code;
   };
   
   // scope is self
   struct [[eosio::table]] token
   {
      asset          supply;             // current supply
      asset          max_supply;         // max supply
      std::string    lot_name;           // lot name
      std::string    lot_description;    // lot name
      std::string    additional_field_1; // any additional information
      std::string    additional_field_2; // any additional information
      std::string    additional_field_3; // any additional information
      name           issuer;             // user who can manage token
      token_id       id;                 // token id
      int            status;             // lot status

      uint64_t primary_key() const { return id; }
   };

   // scope is owner
   struct [[eosio::table]] account
   {
      asset          balance;
      token_id       token_id;

      uint64_t primary_key() const { return token_id; }
   };

private:

   static constexpr uint64_t MAX_SUPPLY_AMOUNT = 100;
   static constexpr uint64_t SUPPLY_AMOUNT = MAX_SUPPLY_AMOUNT;

   typedef eosio::singleton<"token.config"_n, token_config> token_config_singleton;
   typedef eosio::multi_index<"tokens"_n, token> tokens_index;
   typedef eosio::multi_index<"accounts"_n, account> accounts_index;

   token_config_singleton config_for_token;
   tokens_index tokens;

   void add_balance(const eosio::name& owner,
                    token_id token_id, 
                    const asset& value, 
                    const eosio::name& ram_payer);

   void sub_balance(const name& owner, token_id token_id, const asset& value);
};

} // namespace shared_asset
