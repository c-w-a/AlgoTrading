
// written by cwa 21/03/2024

// * caution, LIVE !

#include "sierrachart.h"

SCDLLName("constellations")

SCSFExport scsf_signal(SCStudyInterfaceRef sc)
{	
	// inputs
	SCInputRef i_depth = sc.Input[0];

	// subgraphs
	SCSubgraphRef s_high_pivots_h = sc.Subgraph[0];
	SCSubgraphRef s_high_pivots_l = sc.Subgraph[1];
	SCSubgraphRef s_low_pivots_h = sc.Subgraph[2];
	SCSubgraphRef s_low_pivots_l = sc.Subgraph[3];
	SCSubgraphRef s_c1 = sc.Subgraph[4];
	SCSubgraphRef s_c2 = sc.Subgraph[5];

	// defaults
	if (sc.SetDefaults)
	{
		sc.GraphName = "constellations signal";
		sc.GraphRegion = 0;
		sc.AutoLoop = 1;
		sc.FreeDLL = 0;

		i_depth.Name = "depth of pivot check";
		i_depth.SetInt(1);

		s_high_pivots_h.Name = "higher high pivots";
		s_high_pivots_h.DrawStyle = DRAWSTYLE_TRIANGLE_DOWN;
		s_high_pivots_h.PrimaryColor = RGB(128,128,192); 
		s_high_pivots_h.LineWidth = 5;

		s_high_pivots_l.Name = "lower high pivots";
		s_high_pivots_l.DrawStyle = DRAWSTYLE_TRIANGLE_DOWN;
		s_high_pivots_l.PrimaryColor = RGB(0,128,192); 
		s_high_pivots_l.LineWidth = 5;

		s_low_pivots_h.Name = "higher low pivots";
		s_low_pivots_h.DrawStyle = DRAWSTYLE_TRIANGLE_UP;
		s_low_pivots_h.PrimaryColor = RGB(128,128,192); 
		s_low_pivots_h.LineWidth = 5;

		s_low_pivots_l.Name = "lower low pivots";
		s_low_pivots_l.DrawStyle = DRAWSTYLE_TRIANGLE_UP;
		s_low_pivots_l.PrimaryColor = RGB(0,128,192); 
		s_low_pivots_l.LineWidth = 5;

		s_c1.Name = "s_c1";
		s_c1.DrawStyle = DRAWSTYLE_ARROW_RIGHT;
		s_c1.PrimaryColor = RGB(245,221,11);
		s_c1.LineWidth = 1;

		s_c2.Name = "s_c2";
		s_c2.DrawStyle = DRAWSTYLE_ARROW_RIGHT;
		s_c2.PrimaryColor = RGB(254,186,106);
		s_c2.LineWidth = 1;

		sc.SetPersistentFloat(0, -1.00); // last pivot high
		sc.SetPersistentFloat(1, FLT_MAX); // last pivot low
		sc.SetPersistentInt(0, -1); // s_c1 state
		sc.SetPersistentInt(1, -1); // s_c2 state
		sc.SetPersistentInt(2, -1); // last bar index

		return;
	}

	if (sc.DownloadingHistoricalData)
		return;

	float& r_high_pivots_h = s_high_pivots_h[sc.Index - (i_depth.GetInt() + 1)];
	float& r_high_pivots_l = s_high_pivots_l[sc.Index - (i_depth.GetInt() + 1)];
	float& r_low_pivots_h = s_low_pivots_h[sc.Index - (i_depth.GetInt() + 1)];
	float& r_low_pivots_l = s_low_pivots_l[sc.Index - (i_depth.GetInt() + 1)];
	float& r_c1 = s_c1[sc.Index];
	float& r_c2 = s_c2[sc.Index];

	float& r_last_pivot_high = sc.GetPersistentFloat(0);
	float& r_last_pivot_low = sc.GetPersistentFloat(1);
	int& r_c1state = sc.GetPersistentInt(0);
	int& r_c2state = sc.GetPersistentInt(1);
	int& r_last_bar_index = sc.GetPersistentInt(2);

	// reset vars on reload or new session
	if (sc.BaseDateTimeIn[sc.Index].GetTime() == HMS_TIME(9, 29, 00) || sc.Index == 0)
	{
		r_last_pivot_high = 0;
		r_last_pivot_low = FLT_MAX;
		r_c1state = -1;
		r_c2state = -1;
		r_last_bar_index = sc.Index - 1;
	}

	// once per bar..
	if (sc.Index > r_last_bar_index || sc.IsFullRecalculation)
    {
		// detect new pivots and add to history
		bool new_pivot_high = false;
		bool new_pivot_low = false;
		float new_pivot_high_price;
		float new_pivot_low_price;
		std::string new_pivot_high_id = "";
		std::string new_pivot_low_id = "";
		for (int i = 1; i <= i_depth.GetInt(); i++)
		{
			new_pivot_high = sc.High[sc.Index - (i_depth.GetInt() + 1)] > sc.High[sc.Index - (i_depth.GetInt() + 1) - i] && sc.High[sc.Index - (i_depth.GetInt() + 1)] > sc.High[sc.Index - (i_depth.GetInt() + 1) + i];
			if (!new_pivot_high)
				break;
		}
		for (int i = 1; i <= i_depth.GetInt(); i++)
		{
			new_pivot_low = sc.Low[sc.Index - (i_depth.GetInt() + 1)] < sc.Low[sc.Index - (i_depth.GetInt() + 1) - i] && sc.Low[sc.Index - (i_depth.GetInt() + 1)] < sc.Low[sc.Index - (i_depth.GetInt() + 1) + i];
			if (!new_pivot_low)
				break;
		}
		if (new_pivot_high && !new_pivot_low)
		{
			new_pivot_high_price = sc.High[sc.Index - (i_depth.GetInt() + 1)];
			new_pivot_high_id = (sc.High[sc.Index - (i_depth.GetInt() + 1)] > r_last_pivot_high) ? "HH" : "LH";
			r_last_pivot_high = new_pivot_high_price;
			if (new_pivot_high_id == "HH")
			{
				r_high_pivots_h = new_pivot_high_price;
			}
			else
			{
				r_high_pivots_l = new_pivot_high_price;
			}	
		}
		else if (!new_pivot_high && new_pivot_low)
		{
			new_pivot_low_price = sc.Low[sc.Index - (i_depth.GetInt() + 1)];
			new_pivot_low_id = (sc.Low[sc.Index - (i_depth.GetInt() + 1)] < r_last_pivot_low) ? "LL" : "HL";
			r_last_pivot_low = new_pivot_low_price;
			if (new_pivot_low_id == "HL")
			{
				r_low_pivots_h = new_pivot_low_price;
			}
			else
			{
				r_low_pivots_l = new_pivot_low_price;
			}	
		}

		// constellation I (0, 1)
		if (r_c1state == -1 && new_pivot_high_id != "")
		{
			if (new_pivot_high_id == "HH")
			{
				r_c1state = 0;
			}
		}
		else if (r_c1state == 0 && (new_pivot_low_id != "" || new_pivot_high_id != ""))
		{
			if (new_pivot_low_id == "LL")
			{
				r_c1state = 1;
			}
			else
			{
				r_c1state = -1;
			}
			
		}
		else if (r_c1state == 1 && (new_pivot_low_id != "" || new_pivot_high_id != ""))
		{
			if (new_pivot_high_id == "LH")
			{
				r_c1 = max(sc.Close[sc.Index - (i_depth.GetInt() + 1)], sc.Open[sc.Index - (i_depth.GetInt() + 1)]);
			}
			r_c1state = -1;
		}

		// constellation II (2, 3)
		if (r_c2state == -1 && new_pivot_low_id != "")
		{
			if (new_pivot_low_id == "LL")
			{
				r_c2state = 2;
			}
		}
		else if (r_c2state == 2 && (new_pivot_low_id != "" || new_pivot_high_id != ""))
		{
			if (new_pivot_high_id == "HH")
			{
				r_c2state = 3;
			}
			else
			{
				r_c2state = -1;
			}
		}
		else if (r_c2state == 3 && (new_pivot_low_id != "" || new_pivot_high_id != ""))
		{
			if (new_pivot_low_id == "HL")
			{
				r_c2 = min(sc.Close[sc.Index - (i_depth.GetInt() + 1)], sc.Open[sc.Index - (i_depth.GetInt() + 1)]);
			}
			r_c2state = -1;
		}
        r_last_bar_index = sc.Index;
    }
}

SCSFExport scsf_autotrade(SCStudyInterfaceRef sc)
{
	SCInputRef input_enabled = sc.Input[0];
	SCInputRef trail_step = sc.Input[1];
	SCInputRef stoploss = sc.Input[2];
	SCInputRef i_nyse_session = sc.Input[3];
	SCInputRef i_hk_session  = sc.Input[4];

	if (sc.SetDefaults)
	{
		sc.GraphName = "constellations autotrade";

		input_enabled.Name = "auto trading enabled";
		input_enabled.SetYesNo(1); // ON

		trail_step.Name = "trail step ticks";
		trail_step.SetInt(3);

		stoploss.Name = "stoploss ticks";
		stoploss.SetInt(5);

		i_nyse_session.Name = "trade during NYSE hours?";
		i_nyse_session.SetInt(1);

		i_hk_session.Name = "trade during HK hours?";
		i_hk_session.SetInt(1);

		sc.AutoLoop = 1;
		sc.GraphRegion = 0;
		sc.FreeDLL = 0;
		
		sc.MaximumPositionAllowed = 5;

		sc.SendOrdersToTradeService = true; // LIVE !

		sc.AllowOppositeEntryWithOpposingPositionOrOrders = false;
		sc.SupportAttachedOrdersForTrading = false;
		sc.SupportReversals = false;
		sc.AllowMultipleEntriesInSameDirection = false; 
		sc.UseGUIAttachedOrderSetting = false;
		sc.CancelAllOrdersOnEntriesAndReversals= false;
		sc.AllowEntryWithWorkingOrders = false;
		sc.CancelAllWorkingOrdersOnExit = true;

		sc.AllowOnlyOneTradePerBar = true;
		
		sc.MaintainTradeStatisticsAndTradesData = true;
		
		return;
	}

	if (sc.IsFullRecalculation || sc.DownloadingHistoricalData)
		return;

	// exit function if auto trading is not enabled
	if (!input_enabled.GetYesNo())
		return;

	int& r_orderID = sc.GetPersistentInt(5);

	// position data
	s_SCPositionData PositionData;
	sc.GetTradePosition(PositionData);

	// flatten positions at 4:07pm
	if (sc.BaseDateTimeIn[sc.Index].GetTime() == HMS_TIME(16, 07, 00) && PositionData.PositionQuantity != 0)
		sc.FlattenPosition();	

	if (sc.Index == sc.GetPersistentInt(6) && PositionData.PositionQuantity == 0)
	{
		sc.CancelAllOrders();
		sc.SetPersistentInt(6, 0);
	}

	float LastTradePrice = sc.Close[sc.Index];

	float c1Signal = -1;
	SCFloatArray c1Array;
	sc.GetStudyArrayUsingID(1, 4, c1Array);
	c1Signal = c1Array[sc.Index];
	float c2Signal = -1;
	SCFloatArray c2Array;
	sc.GetStudyArrayUsingID(1, 5, c2Array);
	c2Signal = c2Array[sc.Index];

	// return if position or working orders exist
	if(PositionData.PositionQuantity > 0)
		return;

	int BarTime = sc.BaseDateTimeIn[sc.Index].GetTime();
	
	bool nyse = false;
	bool hk = false;
	if (i_nyse_session.GetInt() == 1)
		nyse = BarTime >= HMS_TIME(9, 30, 0) && BarTime  < HMS_TIME(16, 05, 00);
	if (i_hk_session.GetInt() == 1)
		hk = BarTime >= HMS_TIME(21, 30, 0) || BarTime  < HMS_TIME(4, 0, 0);

    // create new s_SCNewOrder object
    s_SCNewOrder c1Order;
    
    c1Order.OrderType = SCT_ORDERTYPE_STOP;
    c1Order.TextTag = "c1 trade";
    c1Order.Symbol = "GCJ24_FUT_CME";
    c1Order.TimeInForce = SCT_TIF_GOOD_TILL_CANCELED;
    c1Order.AttachedOrderStop1Type = SCT_ORDERTYPE_STEP_TRAILING_STOP;
	c1Order.TrailStopStepPriceAmount = 35 * sc.TickSize;

    // place the order
    if ((nyse || hk) && c1Signal > 0 && sc.Close[sc.Index] < c1Signal)
    {
		c1Order.OrderQuantity = 3;
		c1Order.Price1 = c1Signal;
		c1Order.Stop1Price = c1Signal - 15 * sc.TickSize;
        if (sc.BuyEntry(c1Order) > 0)
        {
            int mainID = c1Order.InternalOrderID;
            r_orderID = mainID;
			sc.SetPersistentInt(6, sc.Index + 7);
        }
    }
    else if ((nyse || hk) && c1Signal > 0 && sc.Close[sc.Index] > c1Signal)
    {
		c1Order.OrderQuantity = 1;
		c1Order.Price1 = c1Signal - 5 * sc.TickSize;
		c1Order.Stop1Price = c1Signal + 15 * sc.TickSize;
        if (sc.SellEntry(c1Order) > 0)
        {
            int mainID = c1Order.InternalOrderID;
            r_orderID = mainID;
			sc.SetPersistentInt(6, sc.Index + 7);
        }
    }

	// create new s_SCNewOrder object
    s_SCNewOrder c2Order;
    
    c2Order.OrderType = SCT_ORDERTYPE_STOP;
    c2Order.TextTag = "c1 trade";
    c2Order.Symbol = "GCJ24_FUT_CME";
    c2Order.TimeInForce = SCT_TIF_GOOD_TILL_CANCELED;
    c2Order.AttachedOrderStop1Type = SCT_ORDERTYPE_STEP_TRAILING_STOP;
	c2Order.TrailStopStepPriceAmount = trail_step.GetInt() * sc.TickSize;

	// place the order
    if ((nyse || hk) && c2Signal > 0 && sc.Close[sc.Index] < c2Signal)
    {
		c2Order.OrderQuantity = 3;
		c2Order.Price1 = c2Signal;
		c2Order.Stop1Price = c2Signal - stoploss.GetInt() * sc.TickSize;
        if (sc.BuyEntry(c2Order) > 0)
        {
            int mainID = c2Order.InternalOrderID;
            r_orderID = mainID;
			sc.SetPersistentInt(6, sc.Index + 7);
        }
    }
    else if ((nyse || hk) && c2Signal > 0 && sc.Close[sc.Index] > c2Signal)
    {
		c2Order.OrderQuantity = 1;
		c2Order.Price1 = c2Signal - 5 * sc.TickSize;
		c2Order.Stop1Price = c2Signal + stoploss.GetInt() * sc.TickSize;
        if (sc.SellEntry(c2Order) > 0)
        {
            int mainID = c2Order.InternalOrderID;
            r_orderID = mainID;
			sc.SetPersistentInt(6, sc.Index + 7);
        }
    }
}