# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from telemetry import benchmark

from measurements import tab_switching
import page_sets


@benchmark.Enabled('has tabs')
class TabSwitchingTop10(benchmark.Benchmark):
  """This test records the MPArch.RWH_TabSwitchPaintDuration histogram.

  The histogram is a measure of the time between when a tab was requested to be
  shown, and when first paint occurred. The script opens 10 pages in different
  tabs, waits for them to load, and then switches to each tab and records the
  metric. The pages were chosen from Alexa top ranking sites.
  """
  test = tab_switching.TabSwitching
  page_set = page_sets.Top10PageSet


@benchmark.Enabled('has tabs')
class TabSwitchingTypical25(benchmark.Benchmark):
  """This test records the MPArch.RWH_TabSwitchPaintDuration histogram.

  The histogram is a measure of the time between when a tab was requested to be
  shown, and when first paint occurred. The script opens 25 pages in different
  tabs, waits for them to load, and then switches to each tab and records the
  metric. The pages were chosen from Alexa top ranking sites.
  """
  test = tab_switching.TabSwitching
  page_set = page_sets.Typical25PageSet


@benchmark.Enabled('has tabs')
class TabSwitchingFiveBlankTabs(benchmark.Benchmark):
  """This test records the MPArch.RWH_TabSwitchPaintDuration histogram.

  The histogram is a measure of the time between when a tab was requested to be
  shown, and when first paint occurred. The script opens 5 blank pages in
  different tabs, waits for them to load, and then switches to each tab and
  records the metric. Blank pages are use to detect unnecessary idle wakeups.
  """
  test = tab_switching.TabSwitching
  page_set = page_sets.FiveBlankPagesPageSet
  options = {'pageset_repeat': 10}


@benchmark.Enabled('has tabs')
class TabSwitchingToughEnergyCases(benchmark.Benchmark):
  """This test records the MPArch.RWH_TabSwitchPaintDuration histogram.

  The histogram is a measure of the time between when a tab was requested to be
  shown, and when first paint occurred. The script opens each page in a
  different tab, waits for them to load, and then switches to each tab and
  records the metric. The pages were written by hand to stress energy usage.
  """
  test = tab_switching.TabSwitching
  page_set = page_sets.ToughEnergyCasesPageSet
  options = {'pageset_repeat': 10}


@benchmark.Disabled  # Just for local testing, not on waterfall.
class TabSwitchingFlashEnergyCases(benchmark.Benchmark):
  test = tab_switching.TabSwitching
  page_set = page_sets.FlashEnergyCasesPageSet
  options = {'pageset_repeat': 10}
