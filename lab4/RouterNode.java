import javax.swing.*;

public class RouterNode {
  private int myID;
  private GuiTextArea myGUI;
  private RouterSimulator sim;
  
  private int infinity = RouterSimulator.INFINITY;
  private int amountNodes = RouterSimulator.NUM_NODES;
  private int[] costs = new int[amountNodes];
  private int[] routes = new int[amountNodes];
  private int[][] table = new int[amountNodes][amountNodes];
  private boolean PoisonReverse = true;
  //--------------------------------------------------
  public RouterNode(int ID, RouterSimulator sim, int[] costs) {
    myID = ID;
    this.sim = sim;
    myGUI =new GuiTextArea("  Output window for Router #"+ ID + "  ");
    
    for (int y = 0; y < amountNodes; ++y){
    	if (y != myID)
    	{
    		for (int x = 0; x < amountNodes; ++x){
    			table[y][x] = (x == y) ? 0 : infinity;
    		}
    	}
    }

    System.arraycopy(costs, 0, table[myID], 0, amountNodes);
    System.arraycopy(costs, 0, this.costs, 0, amountNodes);

    for (int route = 0; route < costs.length; ++route){
    	routes[route] = (costs[route] != infinity) ? route : infinity;
    }
    
    /*
     * For debugging
     */
    if (false)
    {
	    System.out.println(myID);
	    for (int c:costs){
	    	System.out.print(c + " ");
	    }
	    System.out.println();
	    for (int r:routes){
	    	System.out.print(r + " ");
	    }
	    System.out.println();
	    for (int m:table[myID]){
	    	System.out.print(m + " ");
	    }
	    System.out.println("\n");
    }
    /*
     * End of debugging
     */
    
    printDistanceTable();
    
    messageAll();
  }

  //--------------------------------------------------
  public void recvUpdate(RouterPacket pkt) {
	  boolean changesNeighbour = false;
	  
	  int pktFrom = pkt.sourceid;
	  for (int x = 0; x < amountNodes; ++x){
		  // If any changes in neighbouring nodes table, then save the new costs
		  if (table[pktFrom][x] != pkt.mincost[x]){
			  table[pktFrom][x] = pkt.mincost[x];
			  changesNeighbour = true;
		  }
	  }
	  
	  // If changes detected in neighbours
	  if(changesNeighbour){
		  boolean changes = false;
		  
		  for (int node = 0; node < amountNodes; ++node) {
              if(node != myID)
              {
            	  int oldCost = table[myID][node];
            	  int currentCost =  table[routes[node]][node] + table[myID][routes[node]];
            	  
            	  // If any changes made our current route cost not equal to updated current route cost
                  if(oldCost != currentCost)
                  {
                      table[myID][node] = currentCost;
                      changes = true;
                  }
                  

            	  int routeCost = table[myID][node];
            	  int directCost = costs[node];
            	  
                  // If any changes made our route more costly than routing directly
                  if(directCost < routeCost){
                	  table[myID][node] = costs[node];
                      routes[node] = node;
                      changes = true;
                  }


                  for (int x = 0; x < amountNodes; ++x) {
                	  int ourRouteCost = table[myID][x];
                	  int otherRouteCost = table[myID][node] + table[node][x];
                	  
                	  // If any changes made our route cost greater than a route trough another node
                      if (ourRouteCost > otherRouteCost)
                      {
                    	  table[myID][x] = otherRouteCost;
                          routes[x] = routes[node];
                          changes = true;
                      }
                  }
              }
          }
	      if(changes){
	    	  messageAll();
	      }
	  }
	  printDistanceTable();
  }
  

  //--------------------------------------------------
  private void sendUpdate(RouterPacket pkt) {
	  sim.toLayer2(pkt);
  }
  

  //--------------------------------------------------
  public void printDistanceTable() {
	  myGUI.println("Current table for " + myID + "  at time " + sim.getClocktime());
	  
	  myGUI.print("nodes|\t");
	  String rowBreaker = "";
	  for (int x = 0; x < amountNodes; ++x){
		  myGUI.print(x+"\t");
		  rowBreaker += "==========";
	  }
	  myGUI.println();
	  myGUI.println(rowBreaker);
	  
	  for(int y = 0; y < amountNodes; ++y){
		  if (y != myID || true)
		  {
			  myGUI.print("node " + y + "\t");
			  for (int x = 0; x < amountNodes; ++x){
				  myGUI.print(table[y][x] + "\t");
			  }
			  myGUI.println();
		  }
	  }
	  
	  myGUI.println(rowBreaker);

	  myGUI.print("Costs");
	  for (int c : costs){
		  myGUI.print("\t" + c);  
	  }

	  myGUI.println();

	  myGUI.print("Routes");
	  for (int r : routes){
		  String out;
		  if (r == 999){
			  out = "\t-";
		  }
		  else{
			  out = "\t" + r;
		  }
		  myGUI.print(out);
	  }
	  
	  myGUI.println();
	  myGUI.println();

  }

  //--------------------------------------------------
  public void updateLinkCost(int dest, int newcost) {
	  System.out.println("updateLinkCost: " + myID + " -> " + dest + " = " + costs[dest] + " : " + newcost);
	  
	  costs[dest] = newcost;
	  
	  // If we ATM are routing through the changed link
      if (routes[dest] == dest)
          table[myID][dest] = newcost;
      
      // If direct routing less than ATM routing
      if(costs[dest] < table[myID][dest]){
    	  table[myID][dest] = costs[dest];
          routes[dest] = dest;
      }

      for (int x = 0; x < amountNodes; ++x) {
    	  // If ATM routing more than tested route
          if (table[myID][x] > table[myID][dest] + table[dest][x])
          {
        	  table[myID][x] = table[myID][dest] + table[dest][x];
              routes[x] = routes[dest];
          }

      }
      messageAll();
  }
  
  public void messageAll() {
	  for (int n = 0; n < amountNodes; ++n){
		  if (n != myID && costs[n] != infinity){
			  
			  RouterPacket pkt = new RouterPacket(myID, n, table[myID]);
			  
			  // If poison reverse set to true
			  if(PoisonReverse){
				  // Send a table with infinity on destinations if we are routing through receiving node
				  int[] poisonTable = new int[amountNodes];
				  for(int x = 0; x < amountNodes; ++x){
				    	poisonTable[x] = (routes[x] == n) ? infinity : table[myID][x];
				  }
				  pkt = new RouterPacket(myID, n, poisonTable);
			  }
			  
			sendUpdate(pkt);
		  }
	  }
  }
}