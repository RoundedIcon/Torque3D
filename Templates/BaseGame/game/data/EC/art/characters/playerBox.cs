
singleton TSShapeConstructor(PlayerBoxDae)
{
   baseShape = "./playerBox.dae";
};

function PlayerBoxDae::onLoad(%this)
{
   %this.renameSequence("ambient", "StandingRoot");
   %this.addSequence("./playerBox_run.dae", "ForwardRun", "0", "1", "1", "0");
}
