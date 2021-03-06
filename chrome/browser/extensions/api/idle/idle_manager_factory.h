// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_EXTENSIONS_API_IDLE_IDLE_MANAGER_FACTORY_H__
#define CHROME_BROWSER_EXTENSIONS_API_IDLE_IDLE_MANAGER_FACTORY_H__

#include "base/memory/singleton.h"
#include "components/keyed_service/content/browser_context_keyed_service_factory.h"

namespace content {
class BrowserContext;
}  // namespace content

namespace extensions {
class IdleManager;

class IdleManagerFactory : public BrowserContextKeyedServiceFactory {
 public:
  static IdleManager* GetForBrowserContext(content::BrowserContext* context);

  static IdleManagerFactory* GetInstance();

 private:
  friend struct DefaultSingletonTraits<IdleManagerFactory>;

  IdleManagerFactory();
  ~IdleManagerFactory() override;

  // BrowserContextKeyedBaseFactory implementation.
  KeyedService* BuildServiceInstanceFor(
      content::BrowserContext* context) const override;
  content::BrowserContext* GetBrowserContextToUse(
      content::BrowserContext* context) const override;
  bool ServiceIsCreatedWithBrowserContext() const override;
  bool ServiceIsNULLWhileTesting() const override;
};

}  // namespace extensions

#endif  // CHROME_BROWSER_EXTENSIONS_API_IDLE_IDLE_MANAGER_FACTORY_H__
