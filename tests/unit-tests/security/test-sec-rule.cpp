/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2013-2014 Regents of the University of California.
 *
 * This file is part of ndn-cxx library (NDN C++ library with eXperimental eXtensions).
 *
 * ndn-cxx library is free software: you can redistribute it and/or modify it under the
 * terms of the GNU Lesser General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later version.
 *
 * ndn-cxx library is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more details.
 *
 * You should have received copies of the GNU General Public License and GNU Lesser
 * General Public License along with ndn-cxx, e.g., in COPYING.md file.  If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * See AUTHORS.md for complete list of ndn-cxx authors and contributors.
 */

#include "security/sec-rule-specific.hpp"
#include "security/sec-rule-relative.hpp"
#include "security/key-chain.hpp"
#include "identity-management-fixture.hpp"

#include "boost-test.hpp"

namespace ndn {

BOOST_FIXTURE_TEST_SUITE(SecurityTestSecRule, security::IdentityManagementFixture)

BOOST_AUTO_TEST_CASE(SecRuleSpecificTest)
{
  Name rsaIdentity("/SecurityTestSecRule/Basic/Rsa");
  BOOST_REQUIRE(addIdentity(rsaIdentity, RsaKeyParams()));
  Name ecdsaIdentity("/SecurityTestSecRule/Basic/Ecdsa");
  BOOST_REQUIRE(addIdentity(ecdsaIdentity, EcdsaKeyParams()));

  Name dataName("SecurityTestSecRule/Basic");
  Data rsaData(dataName);
  m_keyChain.signByIdentity(rsaData, rsaIdentity);
  Data ecdsaData(dataName);
  m_keyChain.signByIdentity(ecdsaData, ecdsaIdentity);
  Data sha256Data(dataName);
  m_keyChain.signWithSha256(sha256Data);

  shared_ptr<Regex> dataRegex =
    make_shared<Regex>("^<SecurityTestSecRule><Basic>$");
  shared_ptr<Regex> signerRegex =
    make_shared<Regex>("^<SecurityTestSecRule><Basic><><KEY><><>$");

  SecRuleSpecific rule(dataRegex, signerRegex);
  BOOST_CHECK(rule.satisfy(rsaData));
  BOOST_CHECK(rule.satisfy(ecdsaData));
  BOOST_CHECK_EQUAL(rule.satisfy(sha256Data), false);

  BOOST_CHECK(rule.matchSignerName(rsaData));
  BOOST_CHECK(rule.matchSignerName(ecdsaData));
  BOOST_CHECK_EQUAL(rule.matchSignerName(sha256Data), false);
}

BOOST_AUTO_TEST_CASE(SecRuleRelativeTest)
{
  Name rsaIdentity("/SecurityTestSecRule/Basic/Rsa");
  BOOST_REQUIRE(addIdentity(rsaIdentity, RsaKeyParams()));
  Name ecdsaIdentity("/SecurityTestSecRule/Basic/Ecdsa");
  BOOST_REQUIRE(addIdentity(ecdsaIdentity, EcdsaKeyParams()));

  Name dataName("SecurityTestSecRule/Basic");
  Data rsaData(dataName);
  m_keyChain.signByIdentity(rsaData, rsaIdentity);
  Data ecdsaData(dataName);
  m_keyChain.signByIdentity(ecdsaData, ecdsaIdentity);
  Data sha256Data(dataName);
  m_keyChain.signWithSha256(sha256Data);

  SecRuleRelative rule("^(<SecurityTestSecRule><Basic>)$",
                       "^(<SecurityTestSecRule><Basic>)<><KEY><><>$",
                       "==", "\\1", "\\1", true);
  BOOST_CHECK(rule.satisfy(rsaData));
  BOOST_CHECK(rule.satisfy(ecdsaData));
  BOOST_CHECK_EQUAL(rule.satisfy(sha256Data), false);

  BOOST_CHECK(rule.matchSignerName(rsaData));
  BOOST_CHECK(rule.matchSignerName(ecdsaData));
  BOOST_CHECK_EQUAL(rule.matchSignerName(sha256Data), false);
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace ndn
