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

   /**
    * @brief Sets the base token settings such as the symbol for the asset.
    * 
    * @param config - base asset parameter for all shared tokens.
    */
   [[eosio::action]]
   void setconfig(const token_config& config);

   /**
    * @brief Allows `issuer` account to create a token in supply of `maximum_supply`.
    * If validation is successful a new entry in tokens table for token gets created.
    * Called only after calling setconfig.
    * 
    * @param issuer - the account that creates the token, and can manage it,
    * @param status - enumeration for managing token status. Not used by contract, required for backend,
    * @param lot_name - lot name. Not used by contract, required for backend,
    * @param lot_description - lot description. Not used by contract, required for backend,
    * @param additional_field_1 - an additional field for storing any information, for example, 
    *                             a link to an image of a lot. Not used by contract, required for backend,
    * @param additional_field_2 - an additional field for storing any information, for example, 
    *                             a link to an image of a lot. Not used by contract, required for backend,
    * @param additional_field_3 - an additional field for storing any information, for example, 
    *                             a link to an image of a lot. Not used by contract, required for backend.
    */
   [[eosio::action]]
   void create(const name& issuer, 
               int status,
               const std::string& lot_name,
               const std::string& lot_description,
               const std::string& additional_field_1,
               const std::string& additional_field_2,
               const std::string& additional_field_3);

   /**
    * @brief This action issues to `to` account a `quantity` of tokens.
    * Called only by `issuer` account that was specified when calling create.
    * 
    * @param to - the account to issue tokens to,
    * @param token_id - unique token identifier,
    * @param memo - the memo string that accompanies the token issue transaction.
    */
   [[eosio::action]]
   void issue(const name& to, token_id token_id, const std::string& memo);

   /**
    * @brief Allows `from` account to transfer to `to` account the `quantity` tokens.
    * One account is debited and the other is credited with quantity tokens.
    * 
    * @param from - the account to transfer from,
    * @param to - the account to be transferred to,
    * @param token_id - unique token identifier, 
    * @param quantity - the quantity of tokens to be transferred,
    * @param memo - the memo string to accompany the transaction.
    */
   [[eosio::action]]
   void transfer(const name& from, const name& to, token_id token_id, const asset& quantity, const std::string& memo);

   /**
    * @brief seter to update the `status`. Called only by the `issuer` account.
    * 
    * @param token_id - unique token identifier,
    * @param status - new status value
    */
   [[eosio::action]]
   void setstatus(token_id token_id, int status);
   
   /**
    * @brief seter to update the `lot_name`. Called only by the `issuer` account.
    * 
    * @param token_id - unique token identifier,
    * @param lot_name - new lot name value
    */
   [[eosio::action]]
   void setlotname(token_id token_id, const std::string& lot_name);
   
   /**
    * @brief seter to update the `lot_description`. Called only by the `issuer` account.
    * 
    * @param token_id - unique token identifier,
    * @param lot_description - new lot description value
    */
   [[eosio::action]]
   void setlotdesc(token_id token_id, const std::string& lot_description);
   
   /**
    * @brief seter to update the `additional_field_1`. Called only by the `issuer` account.
    * 
    * @param token_id - unique token identifier,
    * @param additional_field_1 - new additional info value
    */
   [[eosio::action]]
   void setaddfield1(token_id token_id, const std::string& additional_field_1);
   
   /**
    * @brief seter to update the `additional_field_2`. Called only by the `issuer` account.
    * 
    * @param token_id - unique token identifier,
    * @param additional_field_2 - new additional info value
    */
   [[eosio::action]]
   void setaddfield2(token_id token_id, const std::string& additional_field_2);
   
   /**
    * @brief seter to update the `additional_field_3`. Called only by the `issuer` account.
    * 
    * @param token_id - unique token identifier,
    * @param additional_field_3 - new additional info value
    */
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
